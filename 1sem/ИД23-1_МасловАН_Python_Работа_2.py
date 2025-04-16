#!/usr/bin/env python
# coding: utf-8

# In[5]:


#Уменьшение прочности с прыжком
#оптимизация прыжков (выбирать самые оптимальные кувшинки (макс расстояние + расстоине до берега)
from PyQt6.QtCore import QPointF, QTimer, QVariantAnimation, QCoreApplication, Qt
from PyQt6.QtGui import QColor, QPainter
from PyQt6.QtWidgets import (
    QApplication,
    QGraphicsItem,
    QGraphicsView,
    QGraphicsScene,
    QGraphicsEllipseItem,
    QGraphicsRectItem,
    QGraphicsLineItem,
    QSlider,
)
from random import sample
from math import sqrt

class Circle(QGraphicsEllipseItem):
    def __init__(self, x, y, r, speed):
        super().__init__(0, 0, r, r)
        self.speed = speed
        self.r = r
        self.setPos(x, y)
        self.weight = sample(range(100, 255), 1)[0]
        self.setBrush(QColor(0, self.weight, 0, 127))
        if self.weight >= 200:
            self.setBrush(QColor(self.weight, 0, 0, 127))
        self.animation = QVariantAnimation(duration=10)
        self.animation.finished.connect(self.create_next_point)

    def create_next_point(self):
        x = self.speed + self.x()
        y = self.y()
        self.setPos(x, y)
        if x>=550:
            return self.scene().removeItem(self)
        self.move_to(x, y)
        
    def move_to(self, x, y):
        self.animation.setStartValue(self.pos())
        self.animation.setEndValue(QPointF(x, y))
        self.animation.start()

    def drown_circle(self):
        self.scene().removeItem(self)
        
class Shore(QGraphicsRectItem):
    def __init__(self, x, y, a, b):
        super().__init__(0, 0, a, b)
        self.setPos(x, y)
        self.setBrush(QColor(200, 200, 0))

class Froggy(QGraphicsEllipseItem):
    def __init__(self, x, y, r):
        super().__init__(0, 0, r, r)
        self.r = r
        self.current_circle = None
        self.next_circle = None
        self.setPos(x, y)
        self.updown = True
        self.weight = 200
        self.distance_limit = 175
        self.setBrush(QColor(100, 0, 0))
        self.painter = QGraphicsLineItem()
        self.painter.setPen(QColor(0, 0, 0))
        self.animation_jump = QVariantAnimation(duration=100)
        self.animation_jump.finished.connect(self.closest_point)
        self.animation = QVariantAnimation(duration=10)
        self.animation.finished.connect(self.create_next_point)

        
    def create_next_point(self):
        colliding = self.collidingItems()
        for item in colliding:
            if isinstance(item, Shore):
                if item.y() <= 400:
                    self.updown = False
                elif item.y() > 400:
                    self.updown = True
        
        if self.next_circle == None:
            y = self.y()  
            x = self.x()
        elif isinstance(self.next_circle, Shore):
            y = self.next_circle.y() 
            x = 250
        else:
            y = self.next_circle.y()
            x = self.next_circle.x()
        if self.current_circle == None or self.current_circle == self.next_circle or self.next_circle == None or isinstance(self.next_circle, Shore) or isinstance(self.current_circle, Shore):
            self.current_circle = self.next_circle
        else:
            if self.current_circle.weight >= self.weight:
                self.scene().removeItem(self.current_circle)
            self.current_circle = self.next_circle
        if x>=500 or x<=-0:
            if self.updown == True:
                y = 625 
            else:
                y = 75
            x = 250
        self.painter.setLine(self.x() + self.r/2, self.y() + self.r/2, x + self.r/2, y + self.r/2)
        self.scene().addItem(self.painter)
        self.setPos(x, y)
        self.move_to(x, y)
        
    def move_to(self, x, y):
        self.animation.setStartValue(self.pos())
        self.animation.setEndValue(QPointF(x, y))
        self.animation.start()
        self.animation_jump.start()

    def closest_point(self):
        min_distance = float('inf')
        close_point = self.current_circle
        for item in self.scene().items():
            if isinstance(item, Circle):
                distance = self.calculate_distance(item.x(), item.y(), self.x(), self.y())
                if distance < min_distance and distance <= abs(self.distance_limit) and item != self.current_circle and (item.x() > 30 or item.x() < 470):
                    if self.updown == True and item.y() < self.y():
                        min_distance = distance
                        close_point = item
                    elif self.updown == False and item.y() > self.y():
                        min_distance = distance
                        close_point = item
            elif isinstance(item, Shore):
                if self.updown == True and item.y() <= 400 and sqrt(((item.y() + 50) - self.y()) ** 2) <= self.distance_limit:
                    close_point = item
                elif self.updown == False and item.y() > 400 and sqrt((item.y() - self.y()) ** 2) <= self.distance_limit:    
                    close_point = item
        self.next_circle = close_point


    def calculate_distance(self, point1_x, point1_y, point2_x, point2_y):
        return sqrt((point1_x - point2_x) ** 2 + (point1_y - point2_y) ** 2)
                

class GraphicView(QGraphicsView):
    def __init__(self, window_width, window_height, time_spawn, circle_speed):
        super().__init__()
        self.circle_speed = circle_speed
        self.setWindowTitle('ИД23-1 Маслов АН')
        scene = QGraphicsScene(self)
        self.setFixedWidth(window_width)
        self.setFixedHeight(window_height)
        self.setScene(scene)
        self.setSceneRect(0, 0, window_width, window_height)
        self.setBackgroundBrush(QColor(173, 216, 230))
        slider_circle_spawn = QSlider(Qt.Orientation.Horizontal, self)
        slider_circle_spawn.setGeometry(5, 5, 100, 20)
        slider_circle_spawn.setMinimum(10)
        slider_circle_spawn.setMaximum(100)
        slider_circle_spawn.setValue(time_spawn)
        slider_circle_speed = QSlider(Qt.Orientation.Horizontal, self)
        slider_circle_speed.setGeometry(5, 30, 100, 20)
        slider_circle_speed.setMinimum(1)
        slider_circle_speed.setMaximum(5)
        slider_circle_speed.setValue(2)
        shore_up = Shore(0, 0, 500, 50)
        shore_down = Shore(0, 650, 500, 50)

        self.scene().addItem(shore_up)
        self.scene().addItem(shore_down)
        timer_circlespawn = QTimer(self, interval=time_spawn, timeout=self.create_circle)
        timer_circlespawn.start()
        self.create_froggy()
        slider_circle_spawn.valueChanged.connect(lambda: timer_circlespawn.setInterval(slider_circle_spawn.value()))
        slider_circle_speed.valueChanged.connect(lambda: self.speed_change(slider_circle_speed.value()))

    def speed_change(self, speed_value):
        self.circle_speed = speed_value
        for item in self.scene().items():
            if isinstance(item, Circle):
                item.speed = speed_value


   
        
    def create_circle(self):
        x = -50
        y = sample(range(80, 620), 1)[0]
        r = 30
        self.circle = Circle(x, y, r, self.circle_speed)
        self.scene().addItem(self.circle)
        self.circle.create_next_point()

    def create_froggy(self):
        x = 250
        y = 675
        r = 20
        self.froggy = Froggy(x, y, r)
        self.scene().addItem(self.froggy)
        self.froggy.create_next_point()

app = QCoreApplication.instance()
if app is None:
    app = QApplication([])

window_width = 500
window_height = 700
time_spawn = 10
circle_radius = 30
circle_speed = 2

view = GraphicView(window_width, window_height, time_spawn, circle_speed)
view.show()
app.exec()


# In[ ]:





# In[ ]:




