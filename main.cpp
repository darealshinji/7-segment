/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018, djcj <djcj@gmx.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/fl_draw.H>

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include <fontconfig/fontconfig.h>

#include "ressources.h"
#include "icon.xpm"

enum {
  MODE_STOPWATCH,
  MODE_TIMER,
  MODE_CLOCK
};

enum {
  CLICK_START,
  CLICK_STOP,
  CLICK_RESET
};

class FontDisplay : public Fl_Widget
{
  void draw();
public:
  FontDisplay(int X, int Y, const char *L=NULL);
  virtual ~FontDisplay() { }
};

class SimpleButton : public Fl_Box
{
public:
  SimpleButton(int X, int Y, int W, int H) : Fl_Box(X, Y, W, H) { }
  virtual ~SimpleButton() { }
  int handle(int event);
};


const char *title = "Clock";
int mode = MODE_CLOCK;
int button_mode = CLICK_START;

Fl_Double_Window *win;
Fl_Spinner *set_h, *set_m, *set_s;
FontDisplay *timebox1, *timebox2;
timeb tb;
int ret = 0;

const double interval_sec = 0.009;  /* 9ms */
const int win_w = 330, win_h = 92;
int pos_x, pos_y;
long time_start;
bool timer_running = false;
bool time_init_set = true;
int time_init_h, time_init_m, time_init_s;
long time_init = 0;


FontDisplay::FontDisplay(int X, int Y, const char *L)
 : Fl_Widget(X,Y,0,0,L)
{
  box(FL_NO_BOX);
  align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  labelfont(FL_FREE_FONT);
  labelsize(48);
  color(FL_RED);
}

void FontDisplay::draw(void) {
  draw_box();
  fl_font(labelfont(), labelsize());
  fl_color(color());
  fl_draw(label(), x()+3, y()+3, w()-6, h()-6, align());
}

int SimpleButton::handle(int event)
{
  if (event == FL_PUSH) {
    /* start the callback when the mouse button
     * was hit, not when it was released */
    do_callback();
    return 1;
  }
  return 0;
}

void set_init(void)
{
  char label[16];
  snprintf(label, sizeof(label), "%02d:%02d:%02d", time_init_h, time_init_m, time_init_s);
  timebox1->copy_label(label);
  timebox2->label(".0");
  time_init = (time_init_h * 3600 + time_init_m * 60 + time_init_s) * 1000;
  time_init_set = true;
}

static void button_cb(Fl_Widget *)
{
  switch (button_mode) {
    case CLICK_START:
      ftime(&tb);
      time_start = tb.time * 1000 + tb.millitm;
      timer_running = true;
      button_mode = CLICK_STOP;
      break;
    case CLICK_STOP:
      timer_running = false;
      button_mode = CLICK_RESET;
      break;
    case CLICK_RESET:
      time_start = 0;
      timer_running = false;
      button_mode = CLICK_START;
      set_init();
      Fl::redraw();
      break;
  }
}

static void close_cb(Fl_Widget *, long p) {
  win->hide();
  ret = p;
}

static void set_timer_cb(Fl_Widget *, void *)
{
  time_init_h = set_h->value();
  time_init_m = set_m->value();
  time_init_s = set_s->value();
  pos_x = win->x_root();
  pos_y = win->y_root();
  win->hide();
}

static void timer_cb(void *)
{
  if (timer_running) {
    ftime(&tb);
    char label1[16];
    char label2[8];

    if (mode == MODE_CLOCK) {
      time_t tt = time(0);
      struct tm *time = localtime(&tt);
      snprintf(label1, sizeof(label1), "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
      snprintf(label2, sizeof(label2), ".%d", tb.millitm/100);
    } else {
      long t = (tb.time * 1000 + tb.millitm) - time_start;
      if (mode == MODE_TIMER) {
        t = time_init - t;
        if (t <= 0) {
          t = 0;
          timer_running = false;
          button_mode = CLICK_RESET;
        }
      }
      snprintf(label1, sizeof(label1), "%02ld:%02ld:%02ld", (t/36000000) % 100, ((t/60000) % 60), ((t/1000) % 60));
      snprintf(label2, sizeof(label2), ".%ld", (t%1000) / 100);
    }

    timebox1->copy_label(label1);
    timebox2->copy_label(label2);
    Fl::redraw();
  }
  Fl::repeat_timeout(interval_sec, timer_cb);
}

int set_timer(void)
{
  Fl::scheme("gtk+");
  Fl::get_system_colors();
  Fl_Window::default_icon(new Fl_RGB_Image(new Fl_Pixmap(icon_xpm)));

  win = new Fl_Double_Window(win_w, win_h, title);
  win->callback(close_cb, 1);
  {
    const int spinw = 56, spinh = 32;

    { Fl_Spinner *o = set_h = new Fl_Spinner(16, 36, spinw, spinh, "hours");
      o->align(FL_ALIGN_TOP_LEFT);
      o->minimum(0);
      o->maximum(99);
      o->format("%02.0f");
      o->step(1);
      o->value(time_init_h); }
    { Fl_Spinner *o = set_m = new Fl_Spinner(26 + spinw, 36, spinw, spinh, "min.");
      o->align(FL_ALIGN_TOP_LEFT);
      o->minimum(0);
      o->maximum(59);
      o->format("%02.0f");
      o->step(1);
      o->value(time_init_m); }
    { Fl_Spinner *o = set_s = new Fl_Spinner(36 + spinw*2, 36, spinw, spinh, "sec.");
      o->align(FL_ALIGN_TOP_LEFT);
      o->minimum(0);
      o->maximum(59);
      o->format("%02.0f");
      o->step(1);
      o->value(time_init_s);
      o->take_focus(); }

#ifdef FL_SPINNER_MOD
    set_h->maximum_size(2);
    set_m->maximum_size(2);
    set_s->maximum_size(2);
    set_h->wrap(0);
    set_m->wrap(0);
    set_s->wrap(0);
#endif

    { Fl_Return_Button *o = new Fl_Return_Button(64 + spinw*3, 36, spinw + 10, spinh, "Set");
      o->clear_visible_focus();
      o->callback(set_timer_cb); }
  }
  pos_x = (Fl::w() - win->w()) / 2;
  pos_y = (Fl::h() - win->h()) / 2;
  win->position(pos_x, pos_y);
  win->end();
  win->show();

  Fl::run();
  return ret;
}

int timer(void)
{
  std::string str;
  const char *uuid;
  char *path;

  str = "/tmp/.font-";
  uuid = Fl_Preferences::newUUID();

  if (uuid) {
    str += uuid;
    str.append(1, '-');
  }

  str += "XXXXXX";
  path = strdup(str.c_str());

  if (mkstemp(path) == -1) {
    free(path);
    path = strdup(str.c_str());
  }

  std::ofstream out(path, std::ios::out|std::ios::binary);

  if (out) {
    out.write(reinterpret_cast<char *>(DSEG7_Classic_Bold_modified_ttf), DSEG7_Classic_Bold_modified_ttf_len);
    out.close();
    FcConfigAppFontAddFile(NULL, reinterpret_cast<FcChar8 *>(path));
  }

  if (win) {
    delete win;
  }

  Fl::set_font(FL_FREE_FONT, "B" /*(Bold)*/ "DSEG7 Classic");
  Fl::background(0, 0, 0);
  Fl_Window::default_icon(new Fl_RGB_Image(new Fl_Pixmap(icon_xpm)));

  win = new Fl_Double_Window(win_w, win_h, title);
  {
    /* background */
    { FontDisplay *o = new FontDisplay(20, 20, "88:88:88");
      o->color(fl_color_average(FL_RED, FL_BLACK, 0.26)); }
    { FontDisplay *o = new FontDisplay(276, 34, "8");
      o->color(fl_color_average(FL_RED, FL_BLACK, 0.26));
      o->labelsize(34); }

    /* foreground */
    timebox1 = new FontDisplay(20, 20, "00:00:00");
    timebox2 = new FontDisplay(276, 34, ".0");
    timebox2->labelsize(34);

    if (mode == MODE_TIMER) {
      set_init();
    }

    if (mode != MODE_CLOCK) {
      { SimpleButton *o = new SimpleButton(0, 0, win_w, win_h);
        o->box(FL_NO_BOX);
        o->clear_visible_focus();
        o->callback(button_cb); }
    }
  }

  if (mode != MODE_TIMER) {
    pos_x = (Fl::w() - win->w()) / 2;
    pos_y = (Fl::h() - win->h()) / 2;
  }
  win->position(pos_x, pos_y);
  win->end();
  win->show();

  Fl::add_timeout(interval_sec, timer_cb);
  int rv = Fl::run();

  win->hide();

  if (out) {
    unlink(path);
    FcConfigAppFontClear(NULL);
  }
  free(path);

  return rv;
}

void print_help(void)
{
#ifndef FLTK_STATIC
  int version = Fl::api_version();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;
#endif

  std::cout << "Copyright (c) 2017-2018  djcj <djcj@gmx.de>\n"
    << "using FLTK version "
#ifdef FLTK_STATIC
    << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION << "." << FL_PATCH_VERSION
#else
    << major << "." << minor << "." << patch
#endif
    << " - http://www.fltk.org\n"
    << "\n"
    << "available options:\n"
    << "  --help, -h     print help\n"
    << "  --timer        start in timer mode\n"
    << "  --stopwatch    start in stopwatch mode\n"
    << "  --clock        start in clock mode\n"
    << std::endl;
}

bool endswith(std::string s1, std::string s2)
{
  size_t l1 = s1.length();
  size_t l2 = s2.length();

  if (s1 == s2 || (l1 > l2 && s1.substr(l1 - l2) == s2)) {
    return true;
  }
  return false;
}

int main(int argc, char *argv[])
{
  std::string self(basename(argv[0]));

  /* set default mode based on application name */
  if (endswith(self, "timer")) {
    mode = MODE_TIMER;
  } else if (endswith(self, "stopwatch")) {
    mode = MODE_STOPWATCH;
  } else if (endswith(self, "clock")) {
    mode = MODE_CLOCK;
  }

  if (argc > 2) {
    /* let's keep it simple for now */
    std::cerr << "error: `" << self << "' takes only 1 argument" << std::endl;
    return 1;
  } else if (argc == 2) {
    std::string arg(argv[1]);

    if (arg == "--help" || arg == "-h") {
      print_help();
      return 0;
    } else if (arg == "--timer") {
      mode = MODE_TIMER;
    } else if (arg == "--stopwatch") {
      mode = MODE_STOPWATCH;
    } else if (arg == "--clock") {
      mode = MODE_CLOCK;
    } else {
      std::cerr << "error: unknown argument `" << arg << "'" << std::endl;
      return 1;
    }
  }

  switch (mode) {
    case MODE_TIMER:
      title = "Timer";
      time_init_h = time_init_m = 0;
      time_init_s = 30;
      ret = set_timer();
      break;
    case MODE_STOPWATCH:
      title = "Stopwatch";
      break;
    case MODE_CLOCK:
      title = "Clock";
      timer_running = true;
      break;
  }

  if (ret == 0) {
    ret = timer();
  }
  return ret;
}

