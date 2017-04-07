__author__ = 'kds'
from kivy.app import App
from kivy.uix.widget import Widget
from kivy.graphics import Color, Ellipse
from kivy.config import Config
from kivy.uix.image import Image
from kivy.uix.label import Label
from kivy.clock import Clock
from kivy.uix.boxlayout import BoxLayout

import time

class TimeApp(App):
    def build(self):
        crudeclock = IncrediblyCrudeClock()
        Clock.schedule_interval(crudeclock.update, 1)
        return crudeclock


class IncrediblyCrudeClock(Label):
    def update(self, *args):
        self.text = time.asctime()

class MyPaintWidget(Widget):

    def on_touch_down(self, touch):
        with self.canvas:
            Color(1, 1, 0)
            d = 100.
            # Ellipse(pos=(touch.x - d / 2, touch.y - d / 2), size=(d, d))
            im = Image(source="glow2.jpg",pos=(touch.x - d / 2, touch.y - d / 2), size=(d,d))
            pass
        pass

    def on_touch_move(self, touch):
        with self.canvas:
            self.canvas.clear()
            d = 100.
            Image(source="glow2.jpg",pos=(touch.x - d / 2, touch.y - d / 2), size=(d,d))
            pass
        pass

    def on_touch_up(self, touch):
        self.canvas.clear()
        pass

class MyPaintApp(BoxLayout, App):

    def __init__(self):
        super(MyPaintApp, self).__init__()
        Config.set('graphics', 'show_cursor', '0')
        Config.write()


    def build(self):
        crudeclock = IncrediblyCrudeClock()
        Clock.schedule_interval(crudeclock.update, 1)
        self.add_widget(MyPaintWidget())
        # Clock.schedule_interval(crudeclock.update, 1)
        return self


if __name__ == '__main__':
    MyPaintApp().run()