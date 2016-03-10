# -*- coding: utf-8 -*-
# @Author: ivytin
# @Date:   2016-03-10 16:16:15
# @Last Modified by:   ivytin
# @Last Modified time: 2016-03-10 16:39:40

def chunked_decode(data):
    newdata = ''
    try:
        while(data != ''):
            # gzip压缩后的数据中间也不可能出现\r\n
            off = int(data[:string.index(data, "\r\n")], 16)
            if off == 0:
                break
            data = data[string.index(data, '\r\n') + 2:]
            newdata = "%s%s" % (newdata, data[:off])
            data = data[off + 2]
    except:
        line = traceback.format_exc()
        # 地址可能有误
        print "Exception! %s" %line
    return newdata

def gzip_decode(data):
    lbuf=StringIO(data)
    with gzip.GzipFile(mode='rb',fileobj=lbuf) as inf:
        return inf.read()

def chunked_and_gzip_in_http_decode(data):
    return gzip_decode(chunked_decode(data))