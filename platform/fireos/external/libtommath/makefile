#Makefile for GCC
#
#Tom St Denis

ifeq ($V,1)
silent=
else
silent=@
endif

%.o: %.c
ifneq ($V,1)
	@echo "   * ${CC} $@"
endif
	${silent} ${CC} -c ${CFLAGS} $^ -o $@

#default files to install
ifndef LIBNAME
   LIBNAME=libtommath.a
endif

coverage: LIBNAME:=-Wl,--whole-archive $(LIBNAME)  -Wl,--no-whole-archive

include makefile.include

LCOV_ARGS=--directory .

#START_INS
OBJECTS=bncore.o bn_error.o bn_fast_mp_invmod.o bn_fast_mp_montgomery_reduce.o bn_fast_s_mp_mul_digs.o \
bn_fast_s_mp_mul_high_digs.o bn_fast_s_mp_sqr.o bn_mp_2expt.o bn_mp_abs.o bn_mp_add.o bn_mp_add_d.o \
bn_mp_addmod.o bn_mp_and.o bn_mp_clamp.o bn_mp_clear.o bn_mp_clear_multi.o bn_mp_cmp.o bn_mp_cmp_d.o \
bn_mp_cmp_mag.o bn_mp_cnt_lsb.o bn_mp_copy.o bn_mp_count_bits.o bn_mp_div_2.o bn_mp_div_2d.o bn_mp_div_3.o \
bn_mp_div.o bn_mp_div_d.o bn_mp_dr_is_modulus.o bn_mp_dr_reduce.o bn_mp_dr_setup.o bn_mp_exch.o \
bn_mp_export.o bn_mp_expt_d.o bn_mp_expt_d_ex.o bn_mp_exptmod.o bn_mp_exptmod_fast.o bn_mp_exteuclid.o \
bn_mp_fread.o bn_mp_fwrite.o bn_mp_gcd.o bn_mp_get_int.o bn_mp_get_long.o bn_mp_get_long_long.o \
bn_mp_grow.o bn_mp_import.o bn_mp_init.o bn_mp_init_copy.o bn_mp_init_multi.o bn_mp_init_set.o \
bn_mp_init_set_int.o bn_mp_init_size.o bn_mp_invmod.o bn_mp_invmod_slow.o bn_mp_is_square.o \
bn_mp_jacobi.o bn_mp_karatsuba_mul.o bn_mp_karatsuba_sqr.o bn_mp_lcm.o bn_mp_lshd.o bn_mp_mod_2d.o \
bn_mp_mod.o bn_mp_mod_d.o bn_mp_montgomery_calc_normalization.o bn_mp_montgomery_reduce.o \
bn_mp_montgomery_setup.o bn_mp_mul_2.o bn_mp_mul_2d.o bn_mp_mul.o bn_mp_mul_d.o bn_mp_mulmod.o bn_mp_neg.o \
bn_mp_n_root.o bn_mp_n_root_ex.o bn_mp_or.o bn_mp_prime_fermat.o bn_mp_prime_is_divisible.o \
bn_mp_prime_is_prime.o bn_mp_prime_miller_rabin.o bn_mp_prime_next_prime.o \
bn_mp_prime_rabin_miller_trials.o bn_mp_prime_random_ex.o bn_mp_radix_size.o bn_mp_radix_smap.o \
bn_mp_rand.o bn_mp_read_radix.o bn_mp_read_signed_bin.o bn_mp_read_unsigned_bin.o bn_mp_reduce_2k.o \
bn_mp_reduce_2k_l.o bn_mp_reduce_2k_setup.o bn_mp_reduce_2k_setup_l.o bn_mp_reduce.o \
bn_mp_reduce_is_2k.o bn_mp_reduce_is_2k_l.o bn_mp_reduce_setup.o bn_mp_rshd.o bn_mp_set.o bn_mp_set_int.o \
bn_mp_set_long.o bn_mp_set_long_long.o bn_mp_shrink.o bn_mp_signed_bin_size.o bn_mp_sqr.o bn_mp_sqrmod.o \
bn_mp_sqrt.o bn_mp_sqrtmod_prime.o bn_mp_sub.o bn_mp_sub_d.o bn_mp_submod.o bn_mp_toom_mul.o \
bn_mp_toom_sqr.o bn_mp_toradix.o bn_mp_toradix_n.o bn_mp_to_signed_bin.o bn_mp_to_signed_bin_n.o \
bn_mp_to_unsigned_bin.o bn_mp_to_unsigned_bin_n.o bn_mp_unsigned_bin_size.o bn_mp_xor.o bn_mp_zero.o \
bn_prime_tab.o bn_reverse.o bn_s_mp_add.o bn_s_mp_exptmod.o bn_s_mp_mul_digs.o bn_s_mp_mul_high_digs.o \
bn_s_mp_sqr.o bn_s_mp_sub.o

#END_INS

$(LIBNAME):  $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	$(RANLIB) $@

#make a profiled library (takes a while!!!)
#
# This will build the library with profile generation
# then run the test demo and rebuild the library.
#
# So far I've seen improvements in the MP math
profiled:
	make CFLAGS="$(CFLAGS) -fprofile-arcs -DTESTING" timing
	./ltmtest
	rm -f *.a *.o ltmtest
	make CFLAGS="$(CFLAGS) -fbranch-probabilities"

#make a single object profiled library
profiled_single:
	perl gen.pl
	$(CC) $(CFLAGS) -fprofile-arcs -DTESTING -c mpi.c -o mpi.o
	$(CC) $(CFLAGS) -DTESTING -DTIMER demo/timing.c mpi.o -lgcov -o ltmtest
	./ltmtest
	rm -f *.o ltmtest
	$(CC) $(CFLAGS) -fbranch-probabilities -DTESTING -c mpi.c -o mpi.o
	$(AR) $(ARFLAGS) $(LIBNAME) mpi.o
	ranlib $(LIBNAME)

install: $(LIBNAME)
	install -d $(DESTDIR)$(LIBPATH)
	install -d $(DESTDIR)$(INCPATH)
	install -m 644 $(LIBNAME) $(DESTDIR)$(LIBPATH)
	install -m 644 $(HEADERS_PUB) $(DESTDIR)$(INCPATH)

test: $(LIBNAME) demo/demo.o
	$(CC) $(CFLAGS) demo/demo.o $(LIBNAME) $(LFLAGS) -o test

test_standalone: $(LIBNAME) demo/demo.o
	$(CC) $(CFLAGS) demo/demo.o $(LIBNAME) $(LFLAGS) -o test

.PHONY: mtest
mtest:
	cd mtest ; $(CC) $(CFLAGS) mtest.c $(LFLAGS) -o mtest

timing: $(LIBNAME)
	$(CC) $(CFLAGS) -DTIMER demo/timing.c $(LIBNAME) $(LFLAGS) -o ltmtest

coveralls: coverage
	cpp-coveralls

# makes the LTM book DVI file, requires tetex, perl and makeindex [part of tetex I think]
docdvi: tommath.src
	cd pics ; MAKE=${MAKE} ${MAKE}
	echo "hello" > tommath.ind
	perl booker.pl
	latex tommath > /dev/null
	latex tommath > /dev/null
	makeindex tommath
	latex tommath > /dev/null

# poster, makes the single page PDF poster
poster: poster.tex
	cp poster.tex poster.bak
	touch --reference=poster.tex poster.bak
	(printf "%s" "\def\fixedpdfdate{"; date +'D:%Y%m%d%H%M%S%:z' -d @$$(stat --format=%Y poster.tex) | sed "s/:\([0-9][0-9]\)$$/'\1'}/g") > poster-deterministic.tex
	printf "%s\n" "\pdfinfo{" >> poster-deterministic.tex
	printf "%s\n" "  /CreationDate (\fixedpdfdate)" >> poster-deterministic.tex
	printf "%s\n}\n" "  /ModDate (\fixedpdfdate)" >> poster-deterministic.tex
	cat poster.tex >> poster-deterministic.tex
	mv poster-deterministic.tex poster.tex
	touch --reference=poster.bak poster.tex
	pdflatex poster
	sed -b -i 's,^/ID \[.*\]$$,/ID [<0> <0>],g' poster.pdf
	mv poster.bak poster.tex
	rm -f poster.aux poster.log poster.out

# makes the LTM book PDF file, requires tetex, cleans up the LaTeX temp files
docs:   docdvi
	dvipdf tommath
	rm -f tommath.log tommath.aux tommath.dvi tommath.idx tommath.toc tommath.lof tommath.ind tommath.ilg
	cd pics ; MAKE=${MAKE} ${MAKE} clean

#LTM user manual
mandvi: bn.tex
	cp bn.tex bn.bak
	touch --reference=bn.tex bn.bak
	(printf "%s" "\def\fixedpdfdate{"; date +'D:%Y%m%d%H%M%S%:z' -d @$$(stat --format=%Y bn.tex) | sed "s/:\([0-9][0-9]\)$$/'\1'}/g") > bn-deterministic.tex
	printf "%s\n" "\pdfinfo{" >> bn-deterministic.tex
	printf "%s\n" "  /CreationDate (\fixedpdfdate)" >> bn-deterministic.tex
	printf "%s\n}\n" "  /ModDate (\fixedpdfdate)" >> bn-deterministic.tex
	cat bn.tex >> bn-deterministic.tex
	mv bn-deterministic.tex bn.tex
	touch --reference=bn.bak bn.tex
	echo "hello" > bn.ind
	latex bn > /dev/null
	latex bn > /dev/null
	makeindex bn
	latex bn > /dev/null

#LTM user manual [pdf]
manual:	mandvi
	pdflatex bn >/dev/null
	sed -b -i 's,^/ID \[.*\]$$,/ID [<0> <0>],g' bn.pdf
	mv bn.bak bn.tex
	rm -f bn.aux bn.dvi bn.log bn.idx bn.lof bn.out bn.toc

pretty:
	perl pretty.build

#zipup the project (take that!)
no_oops: clean
	cd .. ; cvs commit
	echo Scanning for scratch/dirty files
	find . -type f | grep -v CVS | xargs -n 1 bash mess.sh

.PHONY: pre_gen
pre_gen:
	perl gen.pl
	sed -e 's/[[:blank:]]*$$//' mpi.c > pre_gen/mpi.c
	rm mpi.c

zipup:
	rm -rf ../libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH) \
		&& rm -f ../ltm-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).zip ../ltm-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).tar.bz2
	git archive HEAD --prefix=libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)/ > ../libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).tar
	cd .. ; tar xf libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).tar
	MAKE=${MAKE} ${MAKE} -C ../libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH) clean manual poster docs
	tar -c ../libtommath-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)/* | xz -9 > ../ltm-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).tar.xz

new_file:
	bash updatemakes.sh
	perl dep.pl
