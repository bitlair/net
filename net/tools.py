def get_errno(error):
    '''
    Weird function to extract errno from system exceptions such as IOError,
    OSError, socket.error and friends. All of them have their own way of
    returning errors... :(
    '''
    if hasattr(error, 'errno'):
        return error.errno
    elif hasattr(error, 'args') \
        and error.args \
        and isinstance(error.args, tuple) \
        and isinstance(error.args[0], int):
        return error.args[0]
    else:
        return None

