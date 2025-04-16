#!/usr/bin/env python
# coding: utf-8

# In[ ]:


from PyQt6.QtCore import QPointF, QTimer, QVariantAnimation, QCoreApplication
from PyQt6.QtGui import QColor
from PyQt6.QtWidgets import (
    QApplication,
    QGraphicsItem,
    QGraphicsView,
    QGraphicsScene,
    QGraphicsEllipseItem,
    QPushButton,
)
import random
from math import cos, sin, radians

class Circle(QGraphicsEllipseItem):
    def __init__(self, x, y, r, angle):
        super().__init__(x, y, r, r)
        self.angle = angle
        self.diagon = random.sample(range(125, 200), 1)[0]
        self.speed = r/100 + self.diagon/1000
        randomcolor = random.sample(range(255), 3)
        self.setBrush(QColor(randomcolor[0], randomcolor[1], randomcolor[2]))
        self.animation = QVariantAnimation(duration=10)
        self.animation.finished.connect(self.create_next_point)

    def create_next_point(self):
        if self.angle >= 360:
            self.angle = 0
        x = self.diagon * cos(radians(self.angle))
        y = self.diagon * sin(radians(self.angle))
        self.setPos(x, y)
        self.angle += self.speed
        self.move_to(x, y)

    def move_to(self, x, y):
        self.animation.setStartValue(self.pos())
        self.animation.setEndValue(QPointF(x, y))
        self.animation.start()

class GraphicView(QGraphicsView):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('ИД23-1 Маслов АН')
        button_rotateback = QPushButton("↺", self)
        button_rotateback.resize(50,50)
        button_rotate = QPushButton("↻", self)
        button_rotate.resize(50,50)
        button_rotate.move(50,0)
        button_create = QPushButton("Новый круг", self)
        button_create.resize(100,50)
        button_create.move(175,0)
        button_speedup = QPushButton("↑", self)
        button_speedup.resize(50,50)
        button_speedup.move(350,0)
        button_speeddown = QPushButton("↓", self)
        button_speeddown.resize(50,50)
        button_speeddown.move(350,50)
        
        scene = QGraphicsScene(self)
        self.setScene(scene)
        self.setSceneRect(0, 0, 400, 700)
        circle_big = QGraphicsEllipseItem(100,400,200,200)
        circle_big.setBrush(QColor(255, 255, 0))
        self.scene().addItem(circle_big)
        button_create.clicked.connect(self.create_circle)
        button_speedup.clicked.connect(self.angle_change)
        button_speeddown.clicked.connect(self.angle_change)
        button_rotate.clicked.connect(self.angle_change)
        button_rotateback.clicked.connect(self.angle_change)
        
    def create_circle(self):
        angle = 270
        x = 190
        y = 490
        r = random.sample(range(10, 40), 1)[0]
        circle = Circle(x, y, r, angle)
        circle.create_next_point()
        self.scene().addItem(circle)
        
    def angle_change(self):
        for i in self.items():
            if i.__class__== Circle:
                if self.sender().text() == "↺":
                    i.speed = -abs(i.speed)
                elif self.sender().text() == "↻":
                    i.speed = abs(i.speed)
                elif self.sender().text() == "↑":
                    i.speed += 0.5
                elif self.sender().text() == "↓":
                    i.speed -= 0.5

app = QCoreApplication.instance()
if app is None:
    app = QApplication([])
    
view = GraphicView()
view.show()
app.exec()


# In[ ]:





# In[ ]:





# In[ ]:




