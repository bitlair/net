from __future__ import with_statement
from collections import defaultdict
from threading import Lock


class Hookable(object):
    def __init__(self):
        self.hooks = defaultdict(list)
        self.hook_mutex = Lock()

    def hook(self, group, hook, *args, **kwargs):
        with self.hook_mutex:
            self.hooks[group].append((hook, args, kwargs))

    def unhook(self, group, target):
        purge = []
        with self.hook_mutex:
            for hook, args, kwargs in self.hooks:
                if hook == target:
                    purge.append((hook, args, kwargs))
            for hook in purge:
                self.hooks.remove(hook)

    def unhook_group(self, group):
        '''
        Flush all hooks for a given group.
        '''
        self.hooks.pop(group, None)

    def fire(self, group, *args_override, **kwargs_override):
        print 'fire', group
        for hook, args, kwargs in self.hooks.get(group, []):
            args = len(args_override) and args_override or args
            kwargs = len(kwargs_override) and kwargs_override or kwargs
            self.fire_hook(hook, *args, **kwargs)

    def fire_and_forget(self, group, *args_override, **kwargs_override):
        self.fire(group, *args_override, **kwargs_override)
        self.unhook_group(group)

    def fire_hook(self, hook, *args, **kwargs):
        '''
        More or less safe hook execution.
        '''
        try:
            hook(*args, **kwargs)
        except Exception, error:
            #warnings.warn('Exception in hook %r: %r' % (hook, error))
            raise

