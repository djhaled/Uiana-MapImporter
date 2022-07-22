import logging


def setup_logger(name: str) -> logging.Logger:
    """
    Setup logger
    :return:
    """
    try:
        logger = logger
    except:
        logger = logging.getLogger(name)
    finally:
        logger.setLevel(logging.INFO)

        # create file handler which logs even debug messages
        # fh = logging.FileHandler(logfile, mode='w')
        # fh.setLevel(logging.DEBUG)
        # create console handler with a higher log level

        ch = logging.StreamHandler()
        ch.setLevel(logging.DEBUG)
        # create formatter and add it to the handlers
        formatter = logging.Formatter('%(levelname)s | %(filename)s %(lineno)d %(funcName)s | %(message)s')
        # fh.setFormatter(formatter)
        ch.setFormatter(formatter)

        # Remove handlers
        for handler in logger.handlers[:]:
            logger.removeHandler(handler)

        logger.addHandler(ch)

        return logger
