#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: arvin
# @Date:   2016-01-20 15:07:32
# @Last Modified by:   arvin
# @Last Modified time: 2016-01-20 15:44:36

import requests
import re
import time

base_url = 'http://www.devttys0.com/blog/page/'
total_page = 10
web_re = '<h2 class="entry-title"><a href="(.+?)" title=.+? rel="bookmark">(.+?)</a></h2>'
web_pattern = re.compile(web_re)
entrys = list()

for x in xrange(1,11):
    url = base_url + str(x)
    print 'url: ' + url
    resp = requests.get(url).content
    gp = web_pattern.findall(resp)
    for y in xrange(0, len(gp)):
        entrys.append(gp[y])
    # time.sleep(1)

for x in xrange(0, len(entrys)):
    print entrys[x][1], '\t' ,entrys[x][0]
