#!/usr/bin/env python
# $Id$

import matplotlib
matplotlib.use('Agg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_cairo import FigureCanvasCairo as FigureCanvas
from pylab import arange, array, setp
import os
import sys
from datetime import datetime, timedelta
import time
import re

# Date regex: YYYY-MM-DD
DATE_REGEX_LONG = re.compile("^([0-9]{1,4})-([01]?[0-9])-([0-3]?[0-9])$")

# Datetime regex: "HH:MM:SS" (FR format)
DATETIME_REGEX = re.compile("([0-9]{1,2}):([0-9]{2}):([0-9]{2})$")

def text2html(value):

    value = value.replace('&', '&amp;')
    value = value.replace('<', '&lt;')
    value = value.replace('>', '&gt;')

    return value

def parseDatetime(value):

    now = datetime.today()

    split = value.split(' ')
    if len(split) == 2:
        _date, _time = split
    else:
        _date = split[0]
        _time = split[0]

    year = now.year
    day, month, year = (0, 0, 0)
    hour, minute, sec = (0,0,0)

    regs = DATE_REGEX_LONG.match(_date)
    if regs:
        try:
            day = int(regs.group(3))
            month = int(regs.group(2))
            year = int(regs.group(1))
        except ValueError:
            pass

    regs = DATETIME_REGEX.match(_time)
    if regs:
        try:
            hour = int(regs.group(1))
            minute = int(regs.group(2))
            sec = int(regs.group(3))
        except ValueError:
            pass

    return datetime(year, month, day, hour, minute, sec)

class Graph:

    dpi = 70

    colours = ['#B2B6E0', '#C3F4F8', '#5289FF', '#A9482E', '#21FF85', '#E5D587',
               '#E07191', '#74D2F1', '#5BC466', '#92E0DF', '#FFFFFF', '#AFFF54',
               '#C09858', '#FFCB75', '#33ADFF', '#9E4570', '#9AE0A1', '#47BE4F',
               '#CC0099', '#E0DD8D', '#FF8A2B', '#4B5DFF', '#6DF8BE', '#9C56FF',
               '#BE7344', '#CCBE78', '#E0ACD0', '#FF37E1', '#45709E', '#676FFF',
               '#4CAC84', '#35FF1A', '#806170', '#C3BF46', '#E0829A', '#E6CBB7']


    def __init__(self, size):

        self.size = size

    def get_image(self):
        if not self.__image:
            raise Exception('Please call create_img() before!')
        return self.__image

    def build(self):
        """ Callback """
        pass

    def create_img(self, filename):
        fig = Figure()
        canvas = FigureCanvas(fig)

        self.build(fig)

        fig.set_frameon(False) # Background is hidden
        fig.set_size_inches(self.size[0], self.size[1]) # Set size

        fig.savefig(filename, dpi=self.dpi)

class Pie(Graph):

    def __init__(self, size):

        self.entries = []
        self.labels = []
        Graph.__init__(self, size)

    def add_entry(self, label, value):
        """ Add a part in the pie.
            @param label [string] label
            @param value [int] Absolute value.
        """

        self.labels += [str(label),]
        self.entries += [value,]

    def build(self, fig):
        # Set of beautiful colors.

        ax = fig.add_subplot(121)

        self.__pie = ax.pie(self.entries, labels=None, colors=self.colours, autopct=None, shadow=True)

        ax = fig.add_subplot(122)
        self.__legend = ax.legend(self.__pie, ['%s (%d)' % (self.labels[i], self.entries[i]) for i in xrange(len(self.labels))], shadow=True)
        ax.grid(False)
        ax.set_axis_off()

    def get_coords(self):
        # We calculate coords to know where are all labels.
        #return []
        coords = []
        for i in self.__legend.get_texts():

            left, bottom, width, height = i.get_window_extent().get_bounds()
            coords += [[left,
                        self.height - bottom - height,
                        left + width,
                        self.height - bottom]]

        return coords

class Histogram(Graph):

    """
    We can create an Histogram with this syntax:
       >>> test = Histogram((255,255), ["redteam", "greenteam", "blueteam", "purpleteam"])

    test is an histogram with size 255x255, and 4 plots "redteam", "greenteam", "blueteam", "purpleteam"
       >>> test.add_entry(1, (1,2,3,4))
       >>> test.add_entry(2, (11,22,33,44))
       >>> test.create_img("score.png")
    """

    def __init__(self, size, title, plot_labels, legend=False):
        """
           @param size (tuple) image size
           @param plot_labels (list) list of plot labels
        """

        self.size = size
        self.__data = []

        for i in plot_labels:
            self.__data += [[]]
        self.__plots = ()

        self.__plot_labels = plot_labels
        self.__labels = []
        self.__legend = legend

        self.title = title

    def add_entry(self, label, values):
        """
           @param label (string) prout
        """

        if len(values) != len(self.__data):
            raise Exception("Bad call (values = %d, __plot_labels = %d)" % (len(values), len(self.__plot_labels)))

        self.__labels += [str(label)]

        i = 0
        for data in self.__data:
            data += [values[i]]
            i += 1

    def build(self, fig):

        if self.__legend:
            ax = fig.add_subplot(121)
        else:
            ax = fig.add_subplot(111)

        rows = len(self.__data)

        ind = arange(len(self.__labels))/2.0  # the x locations for the groups

        def hex2f(hex):

            return float(int(hex, 16))/255

        cellText = []
        self.__bars = ()
        width = 0.5     # the width of the bars
        yoff = array([0.0] * len(self.__labels)) # the bottom values for stacked bar chart
        for row in xrange(rows):
            colours = []

            # Calculate color of a each bar; We use data value to gradiant from blue to red.
            if len(self.__plot_labels) > 1:
                colours = self.colours[row]
            else:
                for i in self.__data[row]:
                    n = float(i)/max(self.__data[row])
                    colours += [(
                                 (hex2f('c3') + n * (hex2f('e0')-hex2f('c3'))),
                                 (hex2f('82') + (1-n) * (hex2f('f4')-hex2f('82'))),
                                 (hex2f('9a') + (1-n) * (hex2f('f8')-hex2f('9a')))
                                )]

            self.__bars += (ax.bar(ind, self.__data[row], width, bottom=yoff, color=colours)[0],)
            yoff = yoff + self.__data[row]
            cellText.append(['%d' % (x) for x in yoff]) # Labels

        ax.set_xticks(ind+width/2 )
        ax.set_xticklabels(self.__labels)
        ax.set_title(self.title)

        # Make a rotation on absisce labels
        setp(ax.get_xticklabels(), 'rotation', 45,
             'horizontalalignment', 'right', fontsize=8)

        if self.__legend:
            ax = fig.add_subplot(122)
            self.__legend = ax.legend(self.__bars, self.__plot_labels, shadow=True)
            ax.grid(False)
            ax.set_axis_off()

class Commit:

    def __init__(self, revision, user, date, time):

        self.revision = revision
        self.user = user
        self.date = date
        self.time = time
        self.changed_files = []
        self.message = []

class User:

    def __init__(self, name):

        self.name = name
        self.commits = []

    def add_commit(self, commit):

        self.commits += [commit]

class CommitList:

    def __init__(self, name):

        self.name = str(name)
        self.commits = []
        self.users_commits = {}

    def add_commit(self, commit):
        self.commits += [commit]
        if not self.users_commits.has_key(commit.user):
            self.users_commits[commit.user] = []
        self.users_commits[commit.user] += [commit]


def main():

    if len(sys.argv) < 2:
        print 'Syntax: %s <path of log> [output dir]' % sys.argv[0]
        return

    os.system('svn up %s >>/dev/null' % sys.argv[1])
    child = os.popen("svn log -v %s" % sys.argv[1])

    if len(sys.argv) > 2:
        output = sys.argv[2] + '/'
    else:
        output = ''

    data = child.read().split('\n')

    users = {}
    dates = {}
    hours = [CommitList(i) for i in xrange(24)]
    months = {}
    all_commits = []

    REGEXP = re.compile("^r([0-9]{1,4}) \| (.*) \| ([0-9 -]+) ([0-9 :]+) \+([0-9]+) \((.*)\) \| ([0-9]+) (line|lines)$")

    nb_line = 0
    commit_declaration = False
    for line in data:
        regs = REGEXP.match(line)
        if regs:
            revision = int(regs.group(1))
            username = regs.group(2)
            date = regs.group(3)
            commit_time = regs.group(4)
            nb_lines = int(regs.group(7)) + 1

            if not users.has_key(username):
                users[username] = User(username)
            user = users[username]

            commit = Commit(revision, user, date, commit_time)
            all_commits += [commit]

            user.add_commit(commit)

            commit_declaration = True

        elif commit_declaration and len(line) > 0:
            all_commits[-1].changed_files += [line]
        elif len(all_commits) > 0 and nb_lines > 0:
            if len(line) > 0:
                all_commits[-1].message += [line]

            commit_declaration = False
            nb_lines -= 1

        else:
            commit_declaration = False

    all_commits.reverse() # first to last

    for commit in all_commits:

            dt = parseDatetime('%s %s' % (commit.date, commit.time))

            hours[dt.hour].add_commit(commit)

            if not dates:
                now = datetime.today().date()
                d = dt.date()
                while d <= now:
                    dates[d] = CommitList('%s-%d-%d' % (str(d.year)[2:4], d.month, d.day))
                    d = d + timedelta(days=1)

            dates[dt.date()].add_commit(commit)

            if not months:
                now = datetime.today().date()
                d = dt.date()
                while d <= now:
                    months[(d.year, d.month)] = CommitList('%s-%d' % (str(d.year)[2:4], d.month))
                    m = d.month + 1
                    y = d.year
                    if m > 12:
                        m = 1
                        y += 1

                    d = datetime(y, m, 1).date()

            months[(dt.year, dt.month)].add_commit(commit)

    all_commits.reverse() # last to first

    html = file(output + 'stats.html', 'w')
    html.write("""<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
     "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>Stats of commits</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <link rel='stylesheet' href='http://vaginus.org/vaginus.css' type='text/css' />
    <link rel="shortcut icon" type="image/png" href="http://vaginus.org/favicon.png" />
    <script type="text/javascript">

    function showhide(name)
    {
        var elem = document.getElementById(name);

        if (!elem) return;

        if(elem.style.display == '')
            elem.style.display = 'none';
        else
            elem.style.display = '';
    }

    function showall(name)
    {
        var revnum = 1;
        var elem;
        while( (elem = document.getElementById(name + revnum)))
        {
            elem.style.display = '';
            revnum++;
        }
    }

    function hideall(name)
    {
        var revnum = 1;
        var elem;
        while( (elem = document.getElementById(name + revnum)))
        {
            elem.style.display = 'none';
            revnum++;
        }
    }

    </script>
</head>

<body>
<div id="container">
<h1><a href="http://vaginus.org">Stats of commits</a></h1>

<div id="content">""")

    html.write('<p>')

    hours_histo = Histogram((4,3), 'Commits by hour', [i.name for i in users.values()])
    for commits in hours:
        lst = []
        for i in users.values():
            if commits.users_commits.has_key(i):
                lst += [len(commits.users_commits[i])]
            else:
                lst += [0]
        hours_histo.add_entry(commits.name, lst)

    hours_histo.create_img(output + 'hours.png')

    html.write('<img src="hours.png" alt="hours" />')

    months_s = sorted(months.items())
    months_histo = Histogram((9,3), 'Commits by months', [i.name for i in users.values()], legend=True)
    for key, commits in months_s:
        lst = []
        for i in users.values():
            if commits.users_commits.has_key(i):
                lst += [len(commits.users_commits[i])]
            else:
                lst += [0]
        months_histo.add_entry(commits.name, lst)

    months_histo.create_img(output + 'months.png')
    html.write('<img alt="months" src="months.png" />')

    html.write('</p>')
    html.write('<p>')

    dates_histo = Histogram((12,4), 'Commits by day', [i.name for i in users.values()])
    dates_s = sorted(dates.items())
    for key, commits in dates_s:
        lst = []
        for i in users.values():
            if commits.users_commits.has_key(i):
                lst += [len(commits.users_commits[i])]
            else:
                lst += [0]
        dates_histo.add_entry(commits.name, lst)

    dates_histo.create_img(output + 'dates.png')
    html.write('<img alt="dates" src="dates.png" />')

    html.write('</p>')
    html.write('<h2>User commits</h2>')
    html.write('<p>')

    users_pie = Pie((7,3))
    for u in users.values():
        users_pie.add_entry(u.name, len(u.commits))

    users_pie.create_img(output + 'users.png')
    html.write('<img alt="users" src="users.png" />')
    html.write('</p>')

    html.write('<h2>ChangeLog</h2>')

    html.write('<p><a href="javascript:showall(\'r\')">Show all</a>/<a href="javascript:hideall(\'r\')">Hide all</a></p>')

    for i in all_commits:
        html.write('<p><b>r%d</b> by <i>%s</i> at %s %s:  (<a href="javascript:showhide(\'r%d\')">Show/hide</a>)</p>' % (i.revision, i.user.name, i.date, i.time, i.revision))
        html.write('<pre id="r%d">' % i.revision)
        for line in i.changed_files:
            html.write('%s\n' % text2html(line))
        html.write('</pre>')
        html.write('<p>')
        for line in i.message:
            html.write('%s<br />' % text2html(line))
        html.write('------------------------------------------------------------------------')

        html.write('</p>')

    t = time.localtime()

    html.write("""
</div>
<div class="foot">
Generated on %02d/%02d/%02d at %02d:%02d
</div>
</div>
</body>
</html>""" % (t[2], t[1], t[0], t[3], t[4]))


if __name__ == '__main__':
    main()

