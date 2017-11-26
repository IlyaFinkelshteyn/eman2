import pytest
import pyautogui
from PyQt4.QtGui import QPixmap
import os


def pytest_configure(config):
    import EMAN2
    EMAN2._called_from_test = True

def pytest_unconfigure(config):
    import EMAN2
    del EMAN2._called_from_test

@pytest.fixture
def curdir(request):
    return request.fspath.dirname

@pytest.fixture
def main_form(module_name, args=[]):
    module = __import__(module_name)
    if not os.path.isdir(module_name):
        os.mkdir(module_name)
    main_form = module.main(args)
    yield main_form
    main_form.close()

class Win(object):
    def __init__(self):
        self.counter = 0
    
    def cycle(self, qtbot, form, dir, clickButton=None):
        form.raise_()
        form.activateWindow()
        if clickButton:
            qtbot.mouseClick(form, clickButton)
        qtbot.waitForWindowShown(form)
        qtbot.wait(1000)
        fname = '%s.png'%os.path.join(dir,str(self.counter))
        qpxmap = QPixmap.grabWindow(form.winId())
        qtbot.wait(1000)
        qpxmap.save(fname,'png')
        print("Click!: %s"%fname)
        qtbot.wait(800)
        self.counter += 1

@pytest.fixture
def win():
    return Win()
