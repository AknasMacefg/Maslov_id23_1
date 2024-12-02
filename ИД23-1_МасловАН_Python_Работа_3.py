#!/usr/bin/env python
# coding: utf-8

#JSON FILE "ИД23-1_МасловАН_Python_Работа_3.json"

from PyQt6.QtCore import QPointF, QTimer, QVariantAnimation, QCoreApplication, Qt
from PyQt6.QtGui import QIntValidator, QColor
from PyQt6.QtWidgets import (
    QApplication,
    QGraphicsView,
    QGraphicsScene,
    QGraphicsEllipseItem,
    QGraphicsRectItem,
    QGraphicsLineItem,
    QSlider, QLabel, QGroupBox, QFormLayout, QLineEdit, QPushButton
)
from random import sample, randint
from math import sqrt
import json

#CIRCLES CODE
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

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self.froggy = Froggy(event.scenePos().x(), event.scenePos().y(), 20)
            self.scene().addItem(self.froggy)
            view.frog_list.append(self.froggy)
            if len(view.frog_list) == 1:
                view.frog_ui_initial()
            self.froggy.create_next_point()



    def create_next_point(self):
        x = self.speed + self.x()
        y = self.y()
        self.setPos(x, y)
        if x>=550:
            try: 
                return self.scene().removeItem(self)
            except:
                return 0
        self.move_to(x, y)
        
    def move_to(self, x, y):
        self.animation.setStartValue(self.pos())
        self.animation.setEndValue(QPointF(x, y))
        self.animation.start()

    def drown_circle(self):
        view.scene().removeItem(self)

#SHORE OBJECT      
class Shore(QGraphicsRectItem):
    def __init__(self, x, y, a, b):
        super().__init__(0, 0, a, b)
        self.setPos(x, y)
        self.setBrush(QColor(200, 200, 0))

#FROG CODE
class Froggy(QGraphicsEllipseItem):
    def __init__(self, x, y, r):
        super().__init__(0, 0, r, r)
        self.r = r
        self.current_circle = None
        self.next_circle = None
        self.setPos(x, y)
        self.updown = True
        self.weight = froggy_weight
        self.distance_limit = froggy_distance
        self.colour = QColor(randint(1, 255), randint(1, 255), randint(1, 255))
        self.setBrush(self.colour)
        self.painter = QGraphicsLineItem()
        self.painter.setPen(QColor(0, 0, 0))
        self.animation_jump = QVariantAnimation(duration=200)
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
            x = self.x()
        else:
            y = self.next_circle.y()
            x = self.next_circle.x()
        if self.current_circle == None or self.current_circle == self.next_circle or self.next_circle == None or isinstance(self.next_circle, Shore) or isinstance(self.current_circle, Shore):
            self.current_circle = self.next_circle
        else:
            if self.current_circle.weight >= self.weight:
                view.scene().removeItem(self.current_circle)
            self.current_circle = self.next_circle
        if x>=550 or x<=0:
            if self.updown == True:
                y = 670
            else:
                y = 0
            x = 250
        self.painter.setLine(self.x() + self.r/2, self.y() + self.r/2, x + self.r/2, y + self.r/2)
        try:
            self.scene().addItem(self.painter)
            self.setPos(x, y)
            self.move_to(x, y)
        except:
            view.scene().removeItem(self.painter)
        
    def move_to(self, x, y):
        self.animation.setStartValue(self.pos())
        self.animation.setEndValue(QPointF(x, y))
        self.animation.start()
        self.animation_jump.start()

    def closest_point(self):
        min_distance = 0
        close_point = self.current_circle
        for item in view.scene().items():
            if isinstance(item, Circle):
                distance = self.calculate_distance(item.x(), item.y(), self.x(), self.y())
                if distance >= min_distance and distance <= self.distance_limit and item != self.current_circle and (item.x() > 30 or item.x() < 470):
                    if self.updown == True and item.y() < self.y() + 50 and item.x() < self.x()+50 and item.x()>20:
                        min_distance = distance
                        close_point = item
                    elif self.updown == False and item.y() > self.y() - 50 and item.x() < self.x()+50 and item.x()>20:
                        min_distance = distance
                        close_point = item

            if isinstance(item, Shore):
                if self.updown == True and item.y() <= 400 and sqrt((item.y() - self.y()) ** 2) <= self.distance_limit:
                    close_point = item
                    break
                elif self.updown == False and item.y() > 400 and sqrt((item.y() - self.y()) ** 2) <= self.distance_limit:    
                    close_point = item
                    break
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
        self.setFixedHeight(window_height + 200)
        self.setScene(scene)
        self.setSceneRect(0, 0, window_width, window_height)
        self.setBackgroundBrush(QColor(173, 216, 230))

        #RIVER SLIDERS
        slider_circle_spawn = QSlider(Qt.Orientation.Horizontal, self)
        slider_circle_spawn.setGeometry(5, 5, 100, 20)
        slider_circle_spawn.setMinimum(10)
        slider_circle_spawn.setMaximum(100)
        slider_circle_spawn.setValue(time_spawn)
        label_circle_spawn = QLabel(self)
        label_circle_spawn.setStyleSheet('color: black')
        label_circle_spawn.move(110, 5)
        label_circle_spawn.setText("Latency: " + str(slider_circle_spawn.value()) + " ms")
        label_circle_spawn.setFixedWidth(150)
        slider_circle_speed = QSlider(Qt.Orientation.Horizontal, self)
        slider_circle_speed.setGeometry(5, 30, 100, 20)
        slider_circle_speed.setMinimum(1)
        slider_circle_speed.setMaximum(5)
        slider_circle_speed.setValue(2)
        label_circle_speed = QLabel(self)
        label_circle_speed.move(110, 30)
        label_circle_speed.setStyleSheet('color: black')
        label_circle_speed.setText("Speed: " + str(slider_circle_speed.value()))

        #FROG SELECTOR
        
        self.frog_index = 0
        self.frog_list = []
        self.frog_groupbox = QGroupBox('Frog Settings')
        self.form_layout = QFormLayout()
        self.frog_colour = QLabel(self)
        self.frog_weight = QLineEdit(self)
        self.frog_distance = QLineEdit(self)
        self.frog_weight.setValidator(QIntValidator())
        self.frog_distance.setValidator(QIntValidator())
        self.frog_groupbox.setLayout(self.form_layout)
        self.frog_groupbox.move(250, -100)
        self.frog_groupbox.setFixedWidth(200)
        self.frog_groupbox.setFixedHeight(100)
        self.form_layout.addRow('Colour:', self.frog_colour)
        self.form_layout.addRow('Weight:', self.frog_weight)
        self.form_layout.addRow('Distance:', self.frog_distance)
        self.scene().addWidget(self.frog_groupbox)
        self.button_next = QPushButton()
        self.button_next.move(460, -95)
        self.button_next.setFixedHeight(30)
        self.button_next.setFixedWidth(30)
        self.button_next.setText("->")
        self.scene().addWidget(self.button_next)
        self.button_previous = QPushButton()
        self.button_previous.move(460, -60)
        self.button_previous.setFixedHeight(30)
        self.button_previous.setFixedWidth(30)
        self.button_previous.setText("<-")
        self.scene().addWidget(self.button_previous)
        self.button_delete = QPushButton()
        self.button_delete.move(460, -25)
        self.button_delete.setFixedHeight(25)
        self.button_delete.setFixedWidth(25)
        self.button_delete.setText("Del")
        self.scene().addWidget(self.button_delete)

        #TURN POINT FOR FROGS
        shore_up = Shore(0, 0, 500, 50)
        shore_down = Shore(0, 650, 500, 50)
        self.scene().addItem(shore_up)
        self.scene().addItem(shore_down)

        #CODE ITERATIONS
        timer_circlespawn = QTimer(self, interval=time_spawn, timeout= self.create_circle)
        timer_circlespawn.start()
        self.create_froggy()
        self.frog_colour.setStyleSheet(f'background-color: {self.frog_list[self.frog_index].colour.name()}')
        self.frog_index_change()
        self.frog_distance.textChanged.connect(lambda: self.frog_value_changer(self.frog_list[self.frog_index], 'distance'))
        slider_circle_spawn.valueChanged.connect(lambda: timer_circlespawn.setInterval(slider_circle_spawn.value()))
        slider_circle_spawn.valueChanged.connect(lambda: label_circle_spawn.setText("Latency: " + str(slider_circle_spawn.value()) + " ms"))
        slider_circle_speed.valueChanged.connect(lambda: self.speed_change(slider_circle_speed.value()))
        slider_circle_speed.valueChanged.connect(lambda: label_circle_speed.setText("Speed: " + str(slider_circle_speed.value())))
        self.button_next.clicked.connect(self.frog_index_changenext)
        self.button_previous.clicked.connect(self.frog_index_changenext)
        self.button_delete.clicked.connect(self.frog_del)
    
    # UI FUNC
    def frog_ui_initial(self):
        self.frog_groupbox.setEnabled(True)
        self.button_next.setEnabled(True)
        self.button_previous.setEnabled(True)
        self.button_delete.setEnabled(True)
        self.frog_index_change()

    def frog_del(self):
        if len(self.frog_list) == 1:
            self.frog_index = 0
            self.frog_groupbox.setEnabled(False)
            self.button_next.setEnabled(False)
            self.button_previous.setEnabled(False)
            self.button_delete.setEnabled(False)
        self.scene().removeItem(self.frog_list[self.frog_index])
        self.scene().removeItem(self.frog_list[self.frog_index].painter)
        del self.frog_list[self.frog_index]
        if len(self.frog_list) != 0:
            if self.frog_index != 0:
                self.frog_index -= 1
            else:
                self.frog_index = len(self.frog_list) - 1
            self.frog_index_change()
        
    def frog_value_changer(self, parametr, parametrname):
        try:
            if parametrname == 'weight':
                parametr.weight = int(self.sender().text())
            else:
                parametr.distance_limit = int(self.sender().text())
        except:
            self.sender().setText('0')
            if parametrname == 'weight':
                parametr.weight = int(self.sender().text())
            else:
                parametr.distance_limit = int(self.sender().text())
    
    def frog_index_change(self):
        self.frog_colour.setStyleSheet(f'background-color: {self.frog_list[self.frog_index].colour.name()}')
        self.frog_weight.setText(str(self.frog_list[self.frog_index].weight))
        self.frog_distance.setText(str(self.frog_list[self.frog_index].distance_limit))



    def frog_index_changenext(self):
        if self.frog_index + 1 != len(self.frog_list): 
            self.frog_index += 1
            self.frog_index_change()
        elif len(self.frog_list) > 1 and self.frog_index == len(self.frog_list) - 1:
            self.frog_index = 0
            self.frog_index_change()
    
    def frog_index_changeprevious(self):
        if self.frog_index != len(self.frog_list) - 1: 
            self.frog_index -= 1
            self.frog_index_change()
        elif len(self.frog_list) > 1 and self.frog_index == 0:
            self.frog_index = len(self.frog_list) - 1
            self.frog_index_change()


    def speed_change(self, speed_value):
        self.circle_speed = speed_value
        for item in self.scene().items():
            if isinstance(item, Circle):
                item.speed = speed_value

    def create_circle(self):
        x = -50
        y = sample(range(80, 620), 1)[0]
        r = 30
        circle = Circle(x, y, r, self.circle_speed)
        self.scene().addItem(circle)
        circle.setZValue(-1)
        circle.create_next_point()

    def create_froggy(self):
        x = 250
        y = 675
        r = 20
        froggy = Froggy(x, y, r)
        self.scene().addItem(froggy)
        froggy.create_next_point()
        self.frog_list.append(froggy)

#INITIALIZATION CODE
app = QCoreApplication.instance()
if app is None:
    app = QApplication([])


try:
    with open('ИД23-1_МасловАН_Python_Работа_3.json', 'r') as f:
        json_settings = f.read()
        settings = json.loads(json_settings)
        window_width = int(settings["window_width"])
        window_height = int(settings["window_height"])
        time_spawn = int(settings["time_spawn"])
        circle_speed = int(settings["circle_speed"])
        froggy_weight = int(settings["froggy_weight"])
        froggy_distance = int(settings["froggy_distance"])
except:
    window_width = 500
    window_height = 700
    time_spawn = 10
    circle_speed = 2
    froggy_weight = 200
    froggy_distance = 175



view = GraphicView(window_width, window_height, time_spawn, circle_speed)
view.show()
app.exec()

