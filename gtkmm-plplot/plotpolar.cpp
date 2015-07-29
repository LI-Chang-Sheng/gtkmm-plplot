/*
Copyright (C) 2015 Tom Schoonjans

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtkmm-plplot/plotpolar.h>
#include <gtkmm-plplot/exception.h>
#include <gtkmm-plplot/utils.h>
#include <gtkmm-plplot/enums.h>
#include <iostream>
#include <gdkmm/rgba.h>
#include <gdkmm/general.h>
#include <cmath>

using namespace Gtk::PLplot;

PlotPolar::PlotPolar(
  const PlotData2D &_data,
  const Glib::ustring &_axis_title_x,
  const Glib::ustring &_axis_title_y,
  const Glib::ustring &_plot_title,
  const double _plot_width_norm,
  const double _plot_height_norm,
  const double _plot_offset_horizontal_norm,
  const double _plot_offset_vertical_norm) :
  PlotAbstract(_axis_title_x, _axis_title_y, _plot_title,
  _plot_width_norm, _plot_height_norm,
  _plot_offset_horizontal_norm,
  _plot_offset_vertical_norm) {

  add_data(_data);
}

PlotPolar::PlotPolar(const PlotPolar &_source) :
  PlotAbstract(_source) {

  for (auto &iter : _source.plot_data) {
    add_data(*iter);
  }
}

PlotPolar::~PlotPolar() {

}

void PlotPolar::plot_data_modified() {
  //update ranges
  //since we are dealing with polar coordinates, we really need to look only
  //at the maximum x (r here)

  std::vector<PLFLT> max_x;

  for (auto &iter : plot_data) {
    auto iter2 = dynamic_cast<PlotData2D*>(iter);
    std::vector<PLFLT> x = iter2->get_vector_x();
    max_x.push_back(*std::max_element(x.begin(), x.end()));
  }

  max_r = *std::max_element(max_x.begin(), max_x.end());

  plot_data_range_x[0] =
  plot_data_range_x[1] = max_r * M_SQRT2 * 1.2;
  plot_data_range_y[0] = 5.0 * M_PI_4;
  plot_data_range_y[1] = M_PI_4;

  coordinate_transform_world_to_plplot(
    plot_data_range_x[0], plot_data_range_y[0],
    plot_data_range_x[0], plot_data_range_y[0]
  );
  coordinate_transform_world_to_plplot(
    plot_data_range_x[1], plot_data_range_y[1],
    plot_data_range_x[1], plot_data_range_y[1]
  );

  plotted_range_x[0] = plot_data_range_x[0];
  plotted_range_x[1] = plot_data_range_x[1];
  plotted_range_y[0] = plot_data_range_y[0];
  plotted_range_y[1] = plot_data_range_y[1];
  _signal_changed.emit();
}

PlotDataAbstract *PlotPolar::add_data(const PlotDataAbstract &data) {
  PlotData2D *data_copy = nullptr;
  try {
    //ensure our data is PlotData2D
    data_copy = new PlotData2D(dynamic_cast<const PlotData2D &>(data));
  }
  catch (std::bad_cast &e) {
    throw Exception("Gtk::PLplot::PlotPolar::add_data -> data must be of PlotData2D type!");
  }

  //throw error if any of the r values are negative
  std::vector<PLFLT> x = data_copy->get_vector_x();
  if (std::count_if(x.begin(), x.end(), std::bind2nd(std::less<PLFLT>(), double(0.0))) > 0) {
    throw Exception("Gtkmm::Plplot::PlotPolar::add_data -> plot R-values must be  positive");
  }

  plot_data.push_back(data_copy);
  data_copy->signal_changed().connect([this](){_signal_changed.emit();});
  data_copy->signal_data_modified().connect([this](){plot_data_modified();});

  _signal_data_added.emit(data_copy);
  return data_copy;
}

void PlotPolar::coordinate_transform_world_to_plplot(PLFLT x_old, PLFLT y_old, PLFLT *x_new, PLFLT *y_new, PLPointer object) {
  *x_new = x_old * cos(y_old);
  *y_new = x_old * sin(y_old);
}

void PlotPolar::coordinate_transform_world_to_plplot(PLFLT x_old, PLFLT y_old, PLFLT &x_new, PLFLT &y_new) {
    coordinate_transform_world_to_plplot(x_old, y_old, &x_new, &y_new, nullptr);
}

void PlotPolar::coordinate_transform_plplot_to_world(PLFLT x_old, PLFLT y_old, PLFLT *x_new, PLFLT *y_new, PLPointer object) {
  *x_new = sqrt(x_old * x_old + y_old * y_old);
  *y_new = atan2(y_old, x_old);
}

void PlotPolar::coordinate_transform_plplot_to_world(PLFLT x_old, PLFLT y_old, PLFLT &x_new, PLFLT &y_new) {
    coordinate_transform_plplot_to_world(x_old, y_old, &x_new, &y_new, nullptr);
}

void PlotPolar::draw_plot(const Cairo::RefPtr<Cairo::Context> &cr, const int width, const int height) {
  if (!shown)
    return;

  canvas_width = width;
  canvas_height = height;
  plot_width = width * plot_width_norm;
  plot_height= height * plot_height_norm;
  plot_offset_x = width * plot_offset_horizontal_norm;
  plot_offset_y = height * plot_offset_vertical_norm;

  if (pls)
    delete pls;
  pls = new plstream;

  pls->sdev("extcairo");
  pls->spage(0.0, 0.0, plot_width , plot_height, plot_offset_x, plot_offset_y);
  Gdk::Cairo::set_source_rgba(cr, background_color);
  cr->rectangle(plot_offset_x, plot_offset_y, plot_width, plot_height);
  cr->fill();
  cr->save();
  cr->translate(plot_offset_x, plot_offset_y);
  pls->init();

  pls->cmd(PLESC_DEVINIT, cr->cobj());

  change_plstream_color(pls, axes_color);

  //plot the box with its axes
  pls->env(plotted_range_x[0], plotted_range_x[1],
           plotted_range_y[0], plotted_range_y[1],
           1, -2);

  //hook up the coordinate transform
  //this will affect all following PLplot commands!
  pls->stransform(&PlotPolar::coordinate_transform_world_to_plplot, nullptr);

  //additional things to be drawn: circles and lines
  //8 major lines
  for (int i = 0; i < 24; i++) {
    if (i % 6 != 0)
      pls->lsty(LineStyle::SHORT_DASH_SHORT_GAP);
    else
      pls->lsty(LineStyle::CONTINUOUS);
    pls->join(0.0, 0.0, max_r * 1.2, i * 2.0*M_PI/24.0);
  }
  pls->lsty(LineStyle::CONTINUOUS);


  //set the label color
  change_plstream_color(pls, titles_color);

  pls->lab(axis_title_x.c_str(), axis_title_y.c_str(), plot_title.c_str());

  for (auto &iter : plot_data) {
    iter->draw_plot_data(cr, pls);
  }
  cr->restore();

  convert_plplot_to_cairo_coordinates(plotted_range_x[0], plotted_range_y[0],
                                      cairo_range_x[0], cairo_range_y[0]);
  convert_plplot_to_cairo_coordinates(plotted_range_x[1], plotted_range_y[1],
                                      cairo_range_x[1], cairo_range_y[1]);
}

void PlotPolar::set_region(double xmin, double xmax, double ymin, double ymax) {
  if (xmin == xmax && ymin == ymax) {
    //due to signal propagation, this function will actually be called twice on a double-click event,
    //the second time after the plot has already been resized to its normal geometry
    //this condition avoids the warning message...
    return;
  }

  if (xmin < 0 || xmax < 0)
    throw Exception("Gtk::PLplot::Plot2D::set_region -> Invalid arguments");

  coordinate_transform_world_to_plplot(
    xmin, ymin,
    xmin, ymin
  );
  coordinate_transform_world_to_plplot(
    xmax, ymax,
    xmax, ymax
  );

  if (xmin >= xmax || ymin >= ymax /*||
      xmin < plot_data_range_x[0] ||
      xmax > plot_data_range_x[1] ||
      ymin < plot_data_range_y[0] ||
      ymax > plot_data_range_y[1]*/) {
    throw Exception("Gtk::PLplot::Plot2D::set_region -> Invalid arguments");
  }
  plotted_range_x[0] = xmin;
  plotted_range_x[1] = xmax;
  plotted_range_y[0] = ymin;
  plotted_range_y[1] = ymax;

  _signal_changed.emit();
}

PlotAbstract *PlotPolar::clone() const {
  PlotAbstract *my_clone = new PlotPolar(*this);
  if(typeid(*this) != typeid(*my_clone)) {
    throw Exception("Gtk::PLplot::PlotPolar::clone -> Classes that derive from PlotAbstract must implement clone!");
  }
  return my_clone;
}