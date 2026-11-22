#ifndef EDEV_ERRORS_H
#define EDEV_ERRORS_H 

/* Every edev function returns EDEV_OK (0) or a negative error code. */

#define EDEV_OK             0   
#define EDEV_ERR_NACK      -1   /* device did not acknowledge */
#define EDEV_ERR_TIMEOUT   -2   /* bus stuck or device too slow */
#define EDEV_ERR_INVAL     -3   /* bad argument (NULL, wrong length) */
#define EDEV_ERR_IO        -4   /* other bus or hardware failure */
#define EDEV_ERR_NOSUPPORT -5   /* backend does not implement this operation */

#endif /* EDEV_ERRORS_H */