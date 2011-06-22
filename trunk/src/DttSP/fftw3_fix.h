#ifndef _FFTW3_FIX
#define _FFTW3_FIX
static void my_fftw_write_char(char c, void *f) { fputc(c, (FILE *) f); }
#define fftw_export_wisdom_to_file(f) fftw_export_wisdom(my_fftw_write_char, (void*) (f))
#define fftwf_export_wisdom_to_file(f) fftwf_export_wisdom(my_fftw_write_char, (void*) (f))
#define fftwl_export_wisdom_to_file(f) fftwl_export_wisdom(my_fftw_write_char, (void*) (f))

static int my_fftw_read_char(void *f) { return fgetc((FILE *) f); }
#define fftw_import_wisdom_from_file(f) fftw_import_wisdom(my_fftw_read_char, (void*) (f))
#define fftwf_import_wisdom_from_file(f) fftwf_import_wisdom(my_fftw_read_char, (void*) (f))
#define fftwl_import_wisdom_from_file(f) fftwl_import_wisdom(my_fftw_read_char, (void*) (f))
#endif
