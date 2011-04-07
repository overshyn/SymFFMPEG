	echo '/* This file is autogenerated.  Do not edit */' >liboilarray.c.tmp
	echo >>liboilarray.c.tmp
	echo '#include <liboil/liboilfunction.h>' >>liboilarray.c.tmp
	echo >>liboilarray.c.tmp

	grep '^_oil_function_class_' .libs/liboiltmp1.exp | \
	  sed -e 's/ .*//' -e 's/.*/extern OilFunctionClass &;/' >>liboilarray.c.tmp

	echo >>liboilarray.c.tmp
	echo 'OilFunctionClass *_oil_function_class_array[] = {' >>liboilarray.c.tmp

	grep '^_oil_function_class_' .libs/liboiltmp1.exp | \
	  sed -e 's/ .*//' -e 's/.*/  \&&,/' >>liboilarray.c.tmp

	echo '  NULL' >>liboilarray.c.tmp
	echo '};' >>liboilarray.c.tmp
	echo >>liboilarray.c.tmp

	grep '^_oil_function_impl_' .libs/liboiltmp1.exp | \
	  sed -e 's/ .*//' -e 's/.*/extern OilFunctionImpl &;/' >>liboilarray.c.tmp

	echo >>liboilarray.c.tmp
	echo 'OilFunctionImpl *_oil_function_impl_array[] = {' >>liboilarray.c.tmp

	grep '^_oil_function_impl_' .libs/liboiltmp1.exp | \
	  sed -e 's/ .*//' -e 's/.*/  \&&,/' >>liboilarray.c.tmp

	echo '  NULL' >>liboilarray.c.tmp
	echo '};' >>liboilarray.c.tmp
	echo >>liboilarray.c.tmp
	cmp liboilarray.c.tmp liboilarray.c || mv liboilarray.c.tmp liboilarray.c

