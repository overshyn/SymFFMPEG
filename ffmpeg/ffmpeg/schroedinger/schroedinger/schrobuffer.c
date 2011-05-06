
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schrobuffer.h>
#include <schroedinger/schrodebug.h>
#include <liboil/liboil.h>
#include <string.h>

static void schro_buffer_free_mem (SchroBuffer * buffer, void *);
static void schro_buffer_free_subbuffer (SchroBuffer * buffer, void *priv);


SchroBuffer *
schro_buffer_new (void)
{
  SchroBuffer *buffer;

  buffer = schro_malloc0 (sizeof(SchroBuffer));
  buffer->ref_count = 1;
  return buffer;
}

SchroBuffer *
schro_buffer_new_and_alloc (int size)
{
  SchroBuffer *buffer = schro_buffer_new ();

  buffer->data = schro_malloc (size);
  buffer->length = size;
  buffer->free = schro_buffer_free_mem;
  
  SCHRO_DEBUG("%p %i", buffer, size);

  return buffer;
}

SchroBuffer *
schro_buffer_new_with_data (void *data, int size)
{
  SchroBuffer *buffer = schro_buffer_new ();

  buffer->data = data;
  buffer->length = size;

  return buffer;
}

SchroBuffer *
schro_buffer_new_subbuffer (SchroBuffer * buffer, int offset, int length)
{
  SchroBuffer *subbuffer = schro_buffer_new ();

  if (buffer->parent) {
    schro_buffer_ref (buffer->parent);
    subbuffer->parent = buffer->parent;
  } else {
    schro_buffer_ref (buffer);
    subbuffer->parent = buffer;
  }
  subbuffer->data = buffer->data + offset;
  subbuffer->length = length;
  subbuffer->free = schro_buffer_free_subbuffer;

  return subbuffer;
}

SchroBuffer *
schro_buffer_ref (SchroBuffer * buffer)
{
  buffer->ref_count++;
  return buffer;
}

void
schro_buffer_unref (SchroBuffer * buffer)
{
  SCHRO_ASSERT(buffer->ref_count > 0);
  buffer->ref_count--;
  if (buffer->ref_count == 0) {
    SCHRO_DEBUG("free %p", buffer);
    if (buffer->free)
      buffer->free (buffer, buffer->priv);
    schro_free (buffer);
  }
}

static void
schro_buffer_free_mem (SchroBuffer * buffer, void *priv)
{
  schro_free (buffer->data);
}

static void
schro_buffer_free_subbuffer (SchroBuffer * buffer, void *priv)
{
  schro_buffer_unref (buffer->parent);
}

SchroBuffer *
schro_buffer_dup (SchroBuffer * buffer)
{
  SchroBuffer *dup;

  dup = schro_buffer_new_and_alloc (buffer->length);
  oil_memcpy (dup->data, buffer->data, buffer->length);

  return dup;
}

