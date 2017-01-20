def result_ok(**kwargs):
    kwargs['result'] = 'ok'
    return kwargs

def result_error(reason, **kwargs):
    kwargs['result'] = 'error'
    kwargs['reason'] = reason
    return kwargs


