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
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Spinner.H>

#include <iostream>
#include <string>
#include <libgen.h>
#include <sys/timeb.h>
#include <time.h>

/* autogenerated file declaring the following:
 *  unsigned char icon_png
 *  unsigned int icon_png_len
 *  unsigned char dots_png
 *  unsigned int dots_png_len
 *  Fl_PNG_Image *image[10]
 *  Fl_PNG_Image *image_s[10]
 *  static const char *Fl_Spinner_Mod_patch  */
#include "ressources.hpp"

enum {
  MODE_STOPWATCH,
  MODE_TIMER,
  MODE_CLOCK
};
static int mode = MODE_STOPWATCH;

enum {
  CLICK_START,
  CLICK_STOP,
  CLICK_RESET
};
static int button_mode = CLICK_START;

const char *title = "Stopwatch";

static Fl_Double_Window *win;
static Fl_Box *ha, *hb, *ma, *mb, *sa, *sb, *msa, *msb;
static Fl_Spinner *set_h, *set_m, *set_s;
static int pos_x, pos_y;
static bool position_set = false;

static timeb tp;
static long time_start;
static long time_init = 0L;
static int time_init_h, time_init_m, time_init_s;
static bool time_init_set = true;
static bool timer_running = false;
static double interval_sec = 0.009;  /* 9ms */

/* a few function prototypes */
static void set_init(void);
static void set_digit(int n, Fl_Box *b, bool small=false);
int seven_segment();


static void button_cb(Fl_Widget*)
{
  switch (button_mode)
  {
    case CLICK_START:
      ftime(&tp);
      time_start = (long)tp.time * 1000L + (long)tp.millitm;
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

/* set initial time */
static void set_init()
{
  if (mode == MODE_TIMER)
  {
    set_digit(time_init_h/10, ha);
    set_digit(time_init_h%10, hb);
    set_digit(time_init_m/10, ma);
    set_digit(time_init_m%10, mb);
    set_digit(time_init_s/10, sa);
    set_digit(time_init_s%10, sb);
  }
  else
  {
    set_digit(0, ha);
    set_digit(0, hb);
    set_digit(0, ma);
    set_digit(0, mb);
    set_digit(0, sa);
    set_digit(0, sb);
  }
  set_digit(0, msa, true);
  set_digit(0, msb, true);
}

/* Set the digit image; the digit equals its array number.
 * The Fl_PNG_Image arrays are declared in the ressources
 * header file and are generated by script for convenience. */
static void set_digit(int n, Fl_Box *b, bool small)
{
  /* for debugging */
  //if (n!=0 && n!=1 && n!=2 && n!=3 && n!=4 && n!=5 && n!=6 && n!=7 && n!=8 && n!=9) {
  //  std::cerr << "error: set_digit(): `" << n
  //    << "' is not a natural positive integer between 0 and 9" << std::endl;
  //}

  if (small) {
    b->image(image_s[n]);
  } else {
    b->image(image[n]);
  }
}

/* calculate time, set digits, update the window, repeat */
static void seven_segment_cb(void*)
{
  if (timer_running)
  {
    int _ha, _hb, _ma, _mb, _sa, _sb, _msa, _msb;
    ftime(&tp);

    if (mode == MODE_CLOCK)
    {
      time_t timep = time(0);
      struct tm *tm = localtime(&timep);

      _ha  = (int)tm->tm_hour / 10;
      _hb  = (int)tm->tm_hour % 10;
      _ma  = (int)tm->tm_min / 10;
      _mb  = (int)tm->tm_min % 10;
      _sa  = (int)tm->tm_sec / 10;
      _sb  = (int)tm->tm_sec % 10;
      _msa = (int)tp.millitm / 100;
      _msb = ((int)tp.millitm % 100) / 10;
    }
    else
    {
      long t = ((long)tp.time * 1000L + (long)tp.millitm) - time_start;

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

      /* actually the %10 is only necessary if
       * you let the stopwatch run for 100 hours */
      _ha  = (int)(t/36000000L) % 10;

      _hb  = (int)(t/3600000L) % 10;
      _ma  = (int)((t/60000L) % 60L) / 10;
      _mb  = (int)((t/60000L) % 60L) % 10;
      _sa  = (int)((t/1000L) % 60L) / 10;
      _sb  = (int)(t%10000L) / 1000;
      _msa = (int)(t%1000L) / 100;
      _msb = (int)(t%100L) / 10;
    }

    set_digit(_ha, ha);
    set_digit(_hb, hb);
    set_digit(_ma, ma);
    set_digit(_mb, mb);
    set_digit(_sa, sa);
    set_digit(_sb, sb);
    set_digit(_msa, msa, true);
    set_digit(_msb, msb, true);

    Fl::redraw();
  }

  Fl::repeat_timeout(interval_sec, seven_segment_cb);
}

/* set initial time for the timer mode */
static void set_init_cb(void*)
{
  time_init_h = set_h->value();
  time_init_m = set_m->value();
  time_init_s = set_s->value();
  time_init = (time_init_h * 3600L + time_init_m * 60L + time_init_s) * 1000L;
  time_init_set = true;

  /* save positions before closing the window */
  pos_x = win->x_root();
  pos_y = win->y_root();
  win->hide();

  seven_segment();
}

/* main window */
int seven_segment()
{
  /* image sizes: n_.png=29x48, n_s.png=20x33, dots.png=11x48 */

  int winw = 6*29 + 2*11 + 20*2 + 4;
  int winh = 48;
  int offx = 24;
  int offy = 24;
  winw += 2*offx;
  winh += 2*offy;

  /* window icon */
  Fl_PNG_Image win_icon(NULL, icon_png, (int)icon_png_len);
  Fl_Window::default_icon(&win_icon);

  win = new Fl_Double_Window(winw, winh, title);
  {
    if (mode == MODE_TIMER && !time_init_set)
    {
      /* timer mode: set initial values */
      int spinw = 52;
      int spinh = 32;

      set_h = new Fl_Spinner(16, 36, spinw, spinh, "hours");
      set_h->align(FL_ALIGN_TOP_LEFT);
      set_h->minimum(0);
      set_h->maximum(99);
      set_h->format("%02.0f");
      set_h->step(1);
      set_h->value(time_init_h);

      set_m = new Fl_Spinner(26 + spinw, 36, spinw, spinh, "min.");
      set_m->align(FL_ALIGN_TOP_LEFT);
      set_m->minimum(0);
      set_m->maximum(59);
      set_m->format("%02.0f");
      set_m->step(1);
      set_m->value(time_init_m);

      set_s = new Fl_Spinner(36 + spinw*2, 36, spinw, spinh, "sec.");
      set_s->align(FL_ALIGN_TOP_LEFT);
      set_s->minimum(0);
      set_s->maximum(59);
      set_s->format("%02.0f");
      set_s->step(1);
      set_s->value(time_init_s);
      set_s->take_focus();

#if defined(Fl_Spinner_Mod) || (FL_MAJOR_VERSION == 1 && FL_MINOR_VERSION > 3)
      set_h->wrap(0);
      set_m->wrap(0);
      set_s->wrap(0);
#endif

      Fl_Button *b = new Fl_Return_Button(54 + spinw*3, 36, spinw + 10, spinh, "Set");
      b->callback((Fl_Callback *)set_init_cb);
    }
    else
    {
      /* initialize the 7-segment digits */

      Fl::background(0, 0, 0);  /* black */

      ha = new Fl_Box( 0 + offx, offy, 29, 48);
      hb = new Fl_Box(29 + offx, offy, 29, 48);

      Fl_PNG_Image *dots = new Fl_PNG_Image(NULL, dots_png, (int)dots_png_len);
      Fl_Box *dots_a = new Fl_Box(2*29 + offx, offy, 11, 48);
      dots_a->image(dots);

      ma = new Fl_Box(2*29 + 11 + offx, offy, 29, 48);
      mb = new Fl_Box(3*29 + 11 + offx, offy, 29, 48);

      Fl_Box *dots_b = new Fl_Box(4*29 + 11 + offx, offy, 11, 48);
      dots_b->image(dots);

      sa = new Fl_Box(4*29 + 2*11 + offx, offy, 29, 48);
      sb = new Fl_Box(5*29 + 2*11 + offx, offy, 29, 48);

      msa = new Fl_Box(6*29 + 2*11      + 4 + offx, 48 - 33 + offy, 20, 33);
      msb = new Fl_Box(6*29 + 2*11 + 20 + 4 + offx, 48 - 33 + offy, 20, 33);

      set_init();

      if (mode != MODE_CLOCK)
      {
        Fl_Button *b = new Fl_Button(0, 0, winw, winh);
        b->box(FL_NO_BOX);
        b->clear_visible_focus();
        b->callback(button_cb);
      }
    }
  }  /* Fl_Double_Window */

  if (mode != MODE_TIMER || (mode == MODE_TIMER && !time_init_set))
  {
    /* center window on start-up */
    pos_x = (Fl::w() - win->w()) / 2;
    pos_y = (Fl::h() - win->h()) / 2;
    win->position(pos_x, pos_y);
  }
  else if (time_init_set && !position_set)
  {
    /* timer mode: restore previous position after the
     * time was set (do this only once) */
    win->position(pos_x, pos_y);
    position_set = true;
  }

  win->end();
  win->show();

  Fl::add_timeout(interval_sec, seven_segment_cb);
  return Fl::run();
}

void print_help()
{
  /* calculate version string from Fl::api_version() to
   * keep compatibility with dynamic FLTK libraries */
  int version = Fl::api_version();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;

  std::cout << "Copyright (c) 2017  djcj <djcj@gmx.de>\n"
    << "using FLTK version " << major << "." << minor << "." << patch << " - http://www.fltk.org\n"
    << "\n"
    << "available options:\n"
    << "  --help, -h     print help\n"
    << "  --timer        start in timer mode\n"
    << "  --stopwatch    start in stopwatch mode\n"
    << "  --clock        start in clock mode" << std::endl;
#ifdef Fl_Spinner_Mod
  if (major == 1 && minor == 3)
  {
    std::cout << "  --patch        view the patches used on FLTK" << std::endl;
  }
#endif
  std::cout << std::endl;
}

int main(int argc, char *argv[])
{
  std::string self(basename(argv[0]));

  /* set default mode based on application name */
  if (self == "timer" || self == "simple_timer")
  {
    mode = MODE_TIMER;
  }
  else if (self == "stopwatch" || self == "simple_stopwatch")
  {
    mode = MODE_STOPWATCH;
  }
  else if (self == "clock" || self == "simple_clock")
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
    else
#ifdef Fl_Spinner_Mod
    /* satisfy FLTK's LGPL exception 3 if we're using a patched version;
     * Fl_Spinner_Mod_patch is declared in the ressources header and was
     * generated from the patch file using a script */
    if (arg == "--patch")
    {
      std::cout << Fl_Spinner_Mod_patch << std::endl;
      return 0;
    }
    else
#endif
    if (arg == "--timer")
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
    time_init_set = false;
    time_init_h = time_init_m = 0;
    time_init_s = 30;
  }
  else if (mode == MODE_CLOCK)
  {
    title = "Clock";
    timer_running = true;  /* always running in clock mode */
  }

  return seven_segment();
}

