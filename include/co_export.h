
#ifndef CO_EXPORT_H
#define CO_EXPORT_H

#ifdef CO_STATIC_DEFINE
#  define CO_EXPORT
#  define CO_NO_EXPORT
#else
#  ifndef CO_EXPORT
#    ifdef copen_EXPORTS
        /* We are building this library */
#      define CO_EXPORT 
#    else
        /* We are using this library */
#      define CO_EXPORT 
#    endif
#  endif

#  ifndef CO_NO_EXPORT
#    define CO_NO_EXPORT 
#  endif
#endif

#ifndef CO_DEPRECATED
#  define CO_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CO_DEPRECATED_EXPORT
#  define CO_DEPRECATED_EXPORT CO_EXPORT CO_DEPRECATED
#endif

#ifndef CO_DEPRECATED_NO_EXPORT
#  define CO_DEPRECATED_NO_EXPORT CO_NO_EXPORT CO_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CO_NO_DEPRECATED
#    define CO_NO_DEPRECATED
#  endif
#endif

#endif /* CO_EXPORT_H */
