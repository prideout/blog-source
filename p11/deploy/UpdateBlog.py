#!/usr/bin/python

####  pyblog module:

# svn checkout http://python-blogger.googlecode.com/svn/trunk/ pyblog
# cd pyblog/pyblog
# python setup.py install

####  colorama module:

# http://pypi.python.org/pypi/colorama

import shutil
import pyblog
import getpass
import codecs
import os
import glob
import zipfile
import tarfile
from ftplib import FTP
import colorama

colorama.init()

def SendFile(ftp, filename):
    f = open(filename,'rb') 
    ftp.storbinary('STOR ' + os.path.basename(filename), f)

print "Wordpress Login "
passwd = getpass.getpass()

postid = 11
filename = 'Article.txt'
createBackup = True

blog = pyblog.WordPress('http://prideout.net/blog/xmlrpc.php', 'admin', passwd)

# If this can't find the post, it'll throw an exception with a good error message.
# Since it goes uncaught, it aborts the program.  Which is fine.
post = blog.get_post(postid)

#print "Found post %d with the following keys:" % postid
#print '\n'.join(post.keys())

contents = open(filename, 'r').read()
#contents = filter(lambda c: c not in "\r", contents)
print "Slurped up '%s'" % filename

if createBackup:
    backup = codecs.open(filename + '.bak', 'wt', 'utf-8')
    backup.write(post['description'])

post['description'] = contents
publish = False
blog.edit_post(postid, post, publish)

title = \
	colorama.Fore.GREEN + colorama.Back.BLACK + \
	post["title"] + \
	colorama.Fore.RESET + colorama.Back.RESET
	
print "Updated '%s' successfully." % title

print "Opening FTP connection..."

DifferentPassword = False
if DifferentPassword:
    print "FTP Login "
    passwd = getpass.getpass()

ftp = FTP('ftp.prideout.net')
ftp.login('prideout', passwd)
#ftp.retrlines('LIST')
#print
ftp.cwd('prideout.net/blog')
#ftp.retrlines('LIST')
#print

try:
    ftp.mkd('glsw')
except:
    pass

ftp.cwd('glsw')

ftp.retrlines('LIST')

try:
    shutil.rmtree('glsw')
except:
    pass

print "Exporting SVN repo..."
os.system('svn export ../src glsw')

print "Zipping..."

z = zipfile.ZipFile('glsw.zip', 'w')
for name in glob.glob("glsw/*"):
	z.write(name, "glsw/" + os.path.basename(name), zipfile.ZIP_DEFLATED)
z.close()

t = tarfile.TarFile('glsw.tar.gz', 'w')
t.close()

t = tarfile.TarFile.open('glsw.tar.gz', 'w:gz')
for name in glob.glob("glsw/*"):
	t.add(name, "glsw/" + os.path.basename(name), zipfile.ZIP_DEFLATED)
t.close()

folder = \
	colorama.Fore.GREEN + colorama.Back.BLACK + \
	ftp.pwd() + \
	colorama.Fore.RESET + colorama.Back.RESET

print "Sending files over FTP to '%s'..." % folder

SendFile(ftp, "glsw.zip")
SendFile(ftp, "glsw.tar.gz")
SendFile(ftp, "glsw/bstrlib.h")
SendFile(ftp, "glsw/bstrlib.c")
SendFile(ftp, "glsw/glsw.h")
SendFile(ftp, "glsw/glsw.c")

print colorama.Fore.WHITE + colorama.Back.RED
os.system('svn status .. -q')
print colorama.Fore.RESET + colorama.Back.RESET
print "DON'T FORGET TO UPDATE SVN "
