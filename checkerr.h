
#ifndef CHECKERR
#define CHECKERR

#define checkerr(errhp, status) \
  do{ \
    char err_buf[512];    \
    sb4 err_code = 0; 		\
	switch (status)  \
	{ \
		case OCI_NEED_DATA: \
		case OCI_NO_DATA:         \
		case OCI_ERROR:           \
		case OCI_INVALID_HANDLE:             \
		case OCI_STILL_EXECUTING:            \
		case OCI_CONTINUE:          \
			OCIErrorGet ((dvoid *) errhp, (ub4) 1,   \
                    (text *) NULL, (sb4 *) &err_code, \
                    err_buf, (ub4) sizeof(err_buf),  \
                    (ub4) OCI_HTYPE_ERROR);  \
            fprintf(stderr, "Error occured. Message: %s. Code: %d.", err_buf, err_code); \
			/*getchar();*/ exit(-1); \
		case OCI_SUCCESS: \
		case OCI_SUCCESS_WITH_INFO:  \
		default:        \
			break;   \
	}  \
  } while(0) 

#endif  ///CHECKERR                                         
