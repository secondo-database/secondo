import time
from PyQt5.QtWidgets import *
from PyQt5.QtCore import QRunnable
from PyQt5.QtCore import QThreadPool
from PyQt5.QtWidgets import QApplication


class Worker(QRunnable):

    def run(self):
        print("Thread start")
        time.sleep(5)
        print("Thread complete")


app = QApplication([])
worker = Worker()
threadpool = QThreadPool()
threadpool.start(worker)
for i in range(0, 50):
    print('wowowo')
    time.sleep(1)

app.exec_()