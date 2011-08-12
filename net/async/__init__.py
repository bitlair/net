from net.async.multiplexer import Multiplexer

def run():
    return Multiplexer.shared().run()

def stop():
    return Multiplexer.shared().stop()

