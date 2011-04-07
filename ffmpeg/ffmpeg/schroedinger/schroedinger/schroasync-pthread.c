
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schro.h>
#include <schroedinger/schroasync.h>
#include <schroedinger/schrodebug.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

struct _SchroAsync {
  int n_threads;
  int n_threads_running;
  int n_idle;
  int stop;

  volatile int n_completed;

  pthread_mutex_t mutex;
  pthread_cond_t app_cond;
  pthread_cond_t thread_cond;

  SchroThread *threads;

  void (*task_func)(void *);
  void *task_priv;

  SchroAsyncScheduleFunc schedule;
  void *schedule_closure;

  SchroAsyncCompleteFunc complete;
};

struct _SchroThread {
  pthread_t pthread;
  SchroExecDomain exec_domain;
  SchroAsync *async;
  int busy;
  int index;
};

struct _SchroMutex {
  pthread_mutex_t mutex;
};

static int domain_key_inited;
static pthread_key_t domain_key;

static void * schro_thread_main (void *ptr);

void
schro_async_init (void)
{

}

SchroAsync *
schro_async_new(int n_threads,
    SchroAsyncScheduleFunc schedule,
    SchroAsyncCompleteFunc complete,
    void *closure)
{
  SchroAsync *async;
  pthread_attr_t attr;
  pthread_mutexattr_t mutexattr;
  pthread_condattr_t condattr;
  int i;

  if (n_threads == 0) {
    char *s;

    s = getenv ("SCHRO_THREADS");
    if (s && s[0]) {
      char *end;
      int n;
      n = strtoul (s, &end, 0);
      if (end[0] == 0) {
        n_threads = n;
      }
    }
    if (n_threads == 0) {
#if defined(_WIN32)
      const char *s = getenv("NUMBER_OF_PROCESSORS");
      if (s) {
        n_threads = atoi(s);
      }
#elif defined(__APPLE__)
      {
        int    mib[]    = {CTL_HW, HW_NCPU};
        size_t dataSize =  sizeof(int);

        if (sysctl(mib, 2, &n_threads, &dataSize, NULL, 0)) {
          n_threads = 0;
        }
      }
#else
      n_threads = sysconf(_SC_NPROCESSORS_CONF);
#endif        
    }
    if (n_threads == 0) {
      n_threads = 1;
    }
  }
  async = schro_malloc0 (sizeof(SchroAsync));

  SCHRO_DEBUG("%d", n_threads);
  async->n_threads = n_threads;
  async->threads = schro_malloc0 (sizeof(SchroThread) * (n_threads + 1));

  async->schedule = schedule;
  async->schedule_closure = closure;
  async->complete = complete;

  pthread_mutexattr_init (&mutexattr);
  pthread_mutex_init (&async->mutex, &mutexattr);
  pthread_condattr_init (&condattr);
  pthread_cond_init (&async->app_cond, &condattr);
  pthread_cond_init (&async->thread_cond, &condattr);

  if (!domain_key_inited) {
    pthread_key_create (&domain_key, NULL);
    domain_key_inited = TRUE;
  }

  pthread_attr_init (&attr);
  
  pthread_mutex_lock (&async->mutex);

  for(i=0;i<n_threads;i++){
    SchroThread *thread = async->threads + i;

    thread->async = async;
    thread->index = i;
    thread->exec_domain = SCHRO_EXEC_DOMAIN_CPU;
    pthread_create (&async->threads[i].pthread, &attr,
        schro_thread_main, async->threads + i);
    pthread_mutex_lock (&async->mutex);
  }
  pthread_mutex_unlock (&async->mutex);

  pthread_attr_destroy (&attr);
  pthread_mutexattr_destroy (&mutexattr);
  pthread_condattr_destroy (&condattr);

  return async;
}

void
schro_async_free (SchroAsync *async)
{
  int i;

  pthread_mutex_lock (&async->mutex);
  async->stop = TRUE;
  while(async->n_threads_running > 0) {
    pthread_cond_signal (&async->thread_cond);
    pthread_cond_wait (&async->app_cond, &async->mutex);
  }
  pthread_mutex_unlock (&async->mutex);

  for(i=0;i<async->n_threads;i++){
    void *ignore;
    pthread_join (async->threads[i].pthread, &ignore);
  }

  schro_free(async->threads);
  schro_free(async);
}

void
schro_async_run_locked (SchroAsync *async, void (*func)(void *), void *ptr)
{
  SCHRO_ASSERT(async->task_func == NULL);

  async->task_func = func;
  async->task_priv = ptr;

  pthread_cond_signal (&async->thread_cond);
}

int schro_async_get_num_completed (SchroAsync *async)
{
  return async->n_completed;
}

static void
schro_async_dump (SchroAsync *async)
{
  int i;
  for(i=0;i<async->n_threads;i++){
    SchroThread *thread = async->threads + i;

    SCHRO_WARNING ("thread %d: busy=%d", i, thread->busy);
  }
}

int
schro_async_wait_locked (SchroAsync *async)
{
#if 1
  struct timespec ts;
  int ret;

#ifdef HAVE_CLOCK_GETTIME
  clock_gettime (CLOCK_REALTIME, &ts);
#else
  {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;
  }
#endif
  ts.tv_sec += 1;
  ret = pthread_cond_timedwait (&async->app_cond, &async->mutex, &ts);
  if (ret != 0) {
    int i;
    for(i=0;i<async->n_threads;i++){
      if (async->threads[i].busy != 0) break;
    }
    if (i == async->n_threads) {
      SCHRO_WARNING("timeout.  deadlock?");
      schro_async_dump (async);
      return FALSE;
    }
  }
  return TRUE;
#else
  pthread_cond_wait (&async->app_cond, &async->mutex);
  return TRUE;
#endif
}

void
schro_async_wait_one (SchroAsync *async)
{
  pthread_mutex_lock (&async->mutex);
  if (async->n_completed > 0) {
    pthread_mutex_unlock (&async->mutex);
    return;
  }

  pthread_cond_wait (&async->app_cond, &async->mutex);
  pthread_mutex_unlock (&async->mutex);
}

void
schro_async_wait (SchroAsync *async, int min_waiting)
{
  if (min_waiting < 1) min_waiting = 1;

  pthread_mutex_lock (&async->mutex);
  if (async->n_completed > 0) {
    pthread_mutex_unlock (&async->mutex);
    return;
  }

  pthread_cond_wait (&async->app_cond, &async->mutex);
  pthread_mutex_unlock (&async->mutex);
}

static void *
schro_thread_main (void *ptr)
{
  void (*func)(void *);
  void *priv;
  SchroThread *thread = ptr;
  SchroAsync *async = thread->async;
  int ret;

  /* thread starts with async->mutex locked */

  pthread_setspecific (domain_key, (void *)(unsigned long)thread->exec_domain);

  async->n_threads_running++;
  thread->busy = FALSE;
  while (1) {
    if (thread->busy == 0) {
      async->n_idle++;
      SCHRO_DEBUG("thread %d: idle", thread->index);
      pthread_cond_wait (&async->thread_cond, &async->mutex);
      SCHRO_DEBUG("thread %d: got signal", thread->index);
      async->n_idle--;
      thread->busy = TRUE;
      if (async->stop) {
        pthread_cond_signal (&async->app_cond);
        async->n_threads_running--;
        pthread_mutex_unlock (&async->mutex);
        SCHRO_DEBUG("thread %d: stopping", thread->index);
        return NULL;
      }
    } else {
      ret = async->schedule (async->schedule_closure, thread->exec_domain);
      /* FIXME ignoring ret */
      if (!async->task_func) {
        thread->busy = FALSE;
        continue;
      }

      thread->busy = TRUE;
      func = async->task_func;
      priv = async->task_priv;
      async->task_func = NULL;

      if (async->n_idle > 0) {
        pthread_cond_signal (&async->thread_cond);
      }
      pthread_mutex_unlock (&async->mutex);

      SCHRO_DEBUG("thread %d: running", thread->index);
      func (priv);
      SCHRO_DEBUG("thread %d: done", thread->index);

      pthread_mutex_lock (&async->mutex);

      async->complete (priv);
    
      pthread_cond_signal (&async->app_cond);
#ifdef HAVE_CUDA
      /* FIXME */
      /* This is required because we don't have a better mechanism
       * for indicating to threads in other exec domains that it is
       * their turn to run.  It's mostly harmless, although causes
       * a lot of unnecessary wakeups in some cases. */
      pthread_cond_broadcast (&async->thread_cond);
#endif

    }
  }
}

void schro_async_lock (SchroAsync *async)
{
  pthread_mutex_lock (&async->mutex);
}

void schro_async_unlock (SchroAsync *async)
{
  pthread_mutex_unlock (&async->mutex);
}

void schro_async_signal_scheduler (SchroAsync *async)
{
  pthread_cond_broadcast (&async->thread_cond);
}

void
schro_async_add_cuda (SchroAsync *async)
{
  SchroThread *thread;
  int i;
  pthread_attr_t attr;

  pthread_mutex_lock (&async->mutex);

  /* We allocated a spare thread structure just for this case. */
  async->n_threads++;
  i = async->n_threads - 1;

  thread = async->threads + i;
  memset (thread, 0, sizeof(SchroThread));

  pthread_attr_init (&attr);

  thread->async = async;
  thread->index = i;
  thread->exec_domain = SCHRO_EXEC_DOMAIN_CUDA;
  pthread_create (&async->threads[i].pthread, &attr,
      schro_thread_main, async->threads + i);
  pthread_mutex_lock (&async->mutex);
  pthread_mutex_unlock (&async->mutex);

  pthread_attr_destroy (&attr);
}

SchroExecDomain
schro_async_get_exec_domain (void)
{
  void *domain;
  domain = pthread_getspecific (domain_key);
  return (int)(unsigned long)domain;
}

SchroMutex *
schro_mutex_new (void)
{
  SchroMutex *mutex;
  pthread_mutexattr_t mutexattr;

  mutex = malloc(sizeof(SchroMutex));
  pthread_mutexattr_init (&mutexattr);
  pthread_mutex_init (&mutex->mutex, &mutexattr);
  pthread_mutexattr_destroy (&mutexattr);

  return mutex;
}

void
schro_mutex_lock (SchroMutex *mutex)
{
  pthread_mutex_lock (&mutex->mutex);
}

void
schro_mutex_unlock (SchroMutex *mutex)
{
  pthread_mutex_unlock (&mutex->mutex);
}

void
schro_mutex_free (SchroMutex *mutex)
{
  pthread_mutex_destroy (&mutex->mutex);
  free (mutex);
}

