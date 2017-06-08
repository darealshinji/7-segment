/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017, djcj <djcj@gmx.de>
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
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/fl_draw.H>

#include <fontconfig/fontconfig.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>

#include "ressources.h"
#include "icon.xpm"

#define FONT_NAME  "B" /*(Bold)*/ "DSEG7 Classic"
#define FONT_FILE  DSEG7_Classic_Bold_modified_ttf
#define FONT_LEN   DSEG7_Classic_Bold_modified_ttf_len


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

const char *title = "Clock";
int mode = MODE_CLOCK;
int button_mode = CLICK_START;

Fl_Double_Window *win;
Fl_Spinner *set_h, *set_m, *set_s;
timeb tb;
int ret = 0;

const double interval_sec = 0.009;  /* 9ms */
const int win_w = 330;
const int win_h = 92;
int pos_x, pos_y;
long time_start;
bool timer_running = false;
bool time_init_set = true;
int time_init_h, time_init_m, time_init_s;
long time_init = 0L;

class FontDisplay : public Fl_Widget {
  void draw();
public:
  int font, size;
  Fl_Color color;

  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char* L = 0) :
    Fl_Widget(X,Y,W,H,L)
  {
    box(B);
    font = 0;
    size = 14;
    color = FL_RED;
  }
};

class SimpleButton : public Fl_Box {
public:
  SimpleButton(int X, int Y, int W, int H, const char *L=0) :
    Fl_Box(X, Y, W, H, L) { }

  virtual ~SimpleButton() { }

  int handle(int event);
};

FontDisplay *timebox1, *timebox2;

void FontDisplay::draw()
{
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(color);
  fl_draw(label(), x()+3, y()+3, w()-6, h()-6, align());
}

int SimpleButton::handle(int event)
{
  switch (event)
  {
    /* start the callback when the mouse button
     * was hit, not when it was released */
    case FL_PUSH:
      do_callback();
      return 1;
    default:
      return 0;
  }
}

std::string random_string(size_t length)
{
  struct timespec ts;
  char ch;
  std::stringstream ss;
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  const size_t max_index = (sizeof(charset) - 1);

  for (size_t i = 0; i < length; ++i)
  {
    timespec_get(&ts, TIME_UTC);
    ch = charset[ts.tv_nsec % max_index];
    ss << ch;
  }
  return ss.str();
}

void set_init()
{
  char *label = new char[9];
  sprintf(label, "%02d:%02d:%02d", time_init_h, time_init_m, time_init_s);
  timebox1->label(label);
  timebox2->label(".0");
  time_init = (time_init_h * 3600L + time_init_m * 60L + time_init_s) * 1000L;
  time_init_set = true;
}

static void button_cb(Fl_Widget*)
{
  switch (button_mode)
  {
    case CLICK_START:
      ftime(&tb);
      time_start = (long)tb.time * 1000L + (long)tb.millitm;
      timer_running = true;
      button_mode = CLICK_STOP;
      break;
    case CLICK_STOP:
      timer_running = false;
      button_mode = CLICK_RESET;
      break;
    case CLICK_RESET:
      time_start = 0L;
      timer_running = false;
      button_mode = CLICK_START;
      set_init();
      Fl::redraw();
      break;
  }
}

static void close_cb(Fl_Widget *, long p)
{
  win->hide();
  ret = (int)p;
}

static void set_timer_cb(void*)
{
  time_init_h = set_h->value();
  time_init_m = set_m->value();
  time_init_s = set_s->value();
  pos_x = win->x_root();
  pos_y = win->y_root();
  win->hide();
}

static void timer_cb(void*)
{
  if (timer_running)
  {
    ftime(&tb);
    char *label1 = new char[9];
    char *label2 = new char[3];

    if (mode == MODE_CLOCK)
    {
      time_t tt = time(0);
      struct tm *time = localtime(&tt);
      sprintf(label1, "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
      sprintf(label2, ".%d", tb.millitm/100);
    }
    else
    {
      long t = ((long)tb.time * 1000L + (long)tb.millitm) - time_start;
      if (mode == MODE_TIMER)
      {
        t = time_init - t;
        if (t <= 0L)
        {
          t = 0L;
          timer_running = false;
          button_mode = CLICK_RESET;
        }
      }
      sprintf(label1, "%02d:%02d:%02d", (int)(t/36000000L) % 100, (int)((t/60000L) % 60L), (int)((t/1000L) % 60L));
      sprintf(label2, ".%d", (int)(t%1000L) / 100);
    }

    timebox1->label(label1);
    timebox2->label(label2);
    Fl::redraw();
  }
  Fl::repeat_timeout(interval_sec, timer_cb);
}

int set_timer()
{
  Fl_RGB_Image win_icon(new Fl_Pixmap(icon_xpm));
  Fl_Window::default_icon(&win_icon);

  Fl::scheme("gtk+");
  Fl::get_system_colors();

  win = new Fl_Double_Window(win_w, win_h, title);
  win->callback(close_cb, 1);
  {
    int spinw = 56;
    int spinh = 32;

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
      o->callback((Fl_Callback *)set_timer_cb); }
  }
  pos_x = (Fl::w() - win->w()) / 2;
  pos_y = (Fl::h() - win->h()) / 2;
  win->position(pos_x, pos_y);
  win->end();
  win->show();

  Fl::run();
  return ret;
}

int timer()
{
  std::string ttf = "/tmp/." + random_string(10) + ".ttf";
  std::ofstream out(ttf.c_str(), std::ios::out|std::ios::binary);

  if (out)
  {
    out.write((char *)FONT_FILE, (std::streamsize)FONT_LEN);
    out.close();
    FcConfigAppFontAddFile(NULL, (const FcChar8 *)ttf.c_str());
  }

  Fl::set_font(FL_FREE_FONT, FONT_NAME);
  Fl::background(0, 0, 0);

  Fl_RGB_Image win_icon(new Fl_Pixmap(icon_xpm));
  Fl_Window::default_icon(&win_icon);

  win = new Fl_Double_Window(win_w, win_h, title);
  {
    /* background */
    { FontDisplay *o = new FontDisplay(FL_NO_BOX, 20, 20, 0, 0, "88:88:88");
      o->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      o->font = FL_FREE_FONT;
      o->color = fl_color_average(FL_RED, FL_BLACK, .26f);
      o->size = 48; }
    { FontDisplay *o = new FontDisplay(FL_NO_BOX, 276, 34, 0, 0, "8");
      o->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      o->font = FL_FREE_FONT;
      o->color = fl_color_average(FL_RED, FL_BLACK, .26f);
      o->size = 34; }

    /* foreground */
    { FontDisplay *o = timebox1 = new FontDisplay(FL_NO_BOX, 20, 20, 0, 0, "00:00:00");
      o->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      o->font = FL_FREE_FONT;
      o->size = 48; }

    { FontDisplay *o = timebox2 = new FontDisplay(FL_NO_BOX, 276, 34, 0, 0, ".0");
      o->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
      o->font = FL_FREE_FONT;
      o->size = 34; }

    if (mode == MODE_TIMER)
    {
      set_init();
    }

    if (mode != MODE_CLOCK)
    {
      { SimpleButton *o = new SimpleButton(0, 0, win_w, win_h);
        o->box(FL_NO_BOX);
        o->clear_visible_focus();
        o->callback(button_cb); }
    }
  }

  if (mode != MODE_TIMER)
  {
    pos_x = (Fl::w() - win->w()) / 2;
    pos_y = (Fl::h() - win->h()) / 2;
  }
  win->position(pos_x, pos_y);
  win->end();
  win->show();

  Fl::add_timeout(interval_sec, timer_cb);
  int ret = Fl::run();

  win->hide();

  if (out)
  {
    unlink(ttf.c_str());
    FcConfigAppFontClear(NULL);
  }

  return ret;
}

void print_help()
{
#ifndef FLTK_STATIC
  int version = Fl::api_version();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;
#endif

  std::cout << "Copyright (c) 2017  djcj <djcj@gmx.de>\n"
    "using FLTK version "
#ifdef FLTK_STATIC
      << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION << "." << FL_PATCH_VERSION
#else
      << major << "." << minor << "." << patch
#endif
      << " - http://www.fltk.org\n"
    "\n"
    "available options:\n"
    "  --help, -h     print help\n"
    "  --timer        start in timer mode\n"
    "  --stopwatch    start in stopwatch mode\n"
    "  --clock        start in clock mode" << std::endl;
  std::cout << std::endl;
}

bool endswith(std::string s1, std::string s2)
{
  size_t l1 = s1.length();
  size_t l2 = s2.length();

  if (s1 == s2 || (l1 > l2 && s1.substr(l1 - l2) == s2))
  {
    return true;
  }
  return false;
}

int main(int argc, char **argv)
{
  std::string self(basename(argv[0]));

  /* set default mode based on application name */
  if (endswith(self, "timer"))
  {
    mode = MODE_TIMER;
  }
  else if (endswith(self, "stopwatch"))
  {
    mode = MODE_STOPWATCH;
  }
  else if (endswith(self, "clock"))
  {
    mode = MODE_CLOCK;
  }

  if (argc > 2)
  {
    /* let's keep it simple for now */
    std::cerr << "error: `" << self << "' takes only 1 argument" << std::endl;
    return 1;
  }
  else if (argc == 2)
  {
    std::string arg(argv[1]);

    if (arg == "--help" || arg == "-h")
    {
      print_help();
      return 0;
    }
    else if (arg == "--timer")
    {
      mode = MODE_TIMER;
    }
    else if (arg == "--stopwatch")
    {
      mode = MODE_STOPWATCH;
    }
    else if (arg == "--clock")
    {
      mode = MODE_CLOCK;
    }
    else
    {
      std::cerr << "error: unknown argument `" << arg << "'" << std::endl;
      return 1;
    }
  }

  if (mode == MODE_TIMER)
  {
    title = "Timer";
    time_init_h = time_init_m = 0;
    time_init_s = 30;
    ret = set_timer();
  }
  else if (mode == MODE_STOPWATCH)
  {
    title = "Stopwatch";
  }
  else if (mode == MODE_CLOCK)
  {
    title = "Clock";
    timer_running = true;
  }

  if (ret == 0)
  {
    ret = timer();
  }
  return ret;
}

