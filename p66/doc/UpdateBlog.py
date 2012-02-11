#!/usr/bin/python

postid = 66
poststr = 'p' + str(postid)

####  pyblog module:
# svn checkout http://python-blogger.googlecode.com/svn/trunk/ pyblog
# cd pyblog/pyblog
# python setup.py install

####  colorama module:
# http://pypi.python.org/pypi/colorama

####  pyenchant module:

# http://www.rfk.id.au/software/pyenchant/download.html
# On Mac, 'sudo port install enchant'
# And for the dictionary:
#
#   wget ftp://ftp.gnu.org/gnu/aspell/dict/en/aspell6-en-6.0-0.tar.bz2
#   tar -xvjf aspell6-en-6.0-0.tar.bz2
#   cd aspell6-en-6.0-0
#   ./configure
#   make
#   sudo make install

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
from enchant.tokenize import get_tokenizer, HTMLChunker
from enchant.checker import SpellChecker
from enchant.tokenize import EmailFilter, URLFilter
import enchant

try: import itertools
except ImportError: mymap, myzip= map, zip
else: mymap, myzip= itertools.imap, itertools.izip

colorama.init()

def SendFile(ftp, pattern):
    for filename in glob.glob(pattern):
        print "Sending " + filename
        f = open(filename,'rb')
        ftp.storbinary('STOR ' + os.path.basename(filename), f)

def formatColumns(string_list, columns, separator=" "):
    "Produce equal-width columns from string_list"
    sublists= []

    # empty_str based on item 0 of string_list
    try:
        empty_str= type(string_list[0])()
    except IndexError: # string_list is empty
        return

    # create a sublist for every column
    for column in xrange(columns):
            sublists.append(string_list[column::columns])

    # find maximum length of a column
    max_sublist_len= max(mymap(len, sublists))

    # make all columns same length
    for sublist in sublists:
         if len(sublist) < max_sublist_len:
             sublist.append(empty_str)

    # calculate a format string for the output lines
    format_str= separator.join(
        "%%-%ds" % max(mymap(len, sublist))
         for sublist in sublists)

    for line_items in myzip(*sublists):
        yield format_str % line_items

print "Wordpress Login "
passwd = getpass.getpass()

filename = 'Post.html'
createBackup = True
spellCheck = True

blog = pyblog.WordPress('http://prideout.net/blog/xmlrpc.php', 'admin', passwd)

# If this can't find the post, it'll throw an exception with a good error message.
# Since it goes uncaught, it aborts the program.  Which is fine.
post = blog.get_post(postid)

#print "Found post %d with the following keys:" % postid
#print '\n'.join(post.keys())

contents = open(filename, 'r').read()
#contents = filter(lambda c: c not in "\r", contents)
print "Slurped up '%s'" % filename

if spellCheck:
    tokenizer = get_tokenizer("en_US",chunkers=(HTMLChunker,))
    words = tokenizer(contents)
    dictionary = enchant.Dict("en_US")
    misspelled = set()
    for word in words:
        if not dictionary.check(word[0]):
            misspelled.add(word[0])
    print colorama.Fore.CYAN + colorama.Back.BLACK
    for line in formatColumns(list(misspelled), 3):
        print line
    print colorama.Fore.RESET + colorama.Back.RESET

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
    ftp.mkd(poststr)
except:
    pass

ftp.cwd(poststr)

ftp.retrlines('LIST')
shutil.rmtree('fluid3d', True)

print "Exporting SVN repo..."
os.system('svn export .. fluid3d')
shutil.rmtree('fluid3d/doc', True)

print "Zipping..."

def zipper(dir, zip_file):
    zip = zipfile.ZipFile(zip_file, 'w', compression=zipfile.ZIP_DEFLATED)
    root_len = len(os.path.abspath(dir))
    for root, dirs, files in os.walk(dir):
        archive_root = os.path.abspath(root)[root_len:]
        for f in files:
            fullpath = os.path.join(root, f)
            archive_name = os.path.join(archive_root, f)
            zip.write(fullpath, archive_name, zipfile.ZIP_DEFLATED)
    zip.close()
    return zip_file

zipper("fluid3d", 'fluid3d.zip')

folder = \
	colorama.Fore.GREEN + colorama.Back.BLACK + \
	ftp.pwd() + \
	colorama.Fore.RESET + colorama.Back.RESET

print "Sending files over FTP to '%s'..." % folder

if True:
    SendFile(ftp, "*.png")
    SendFile(ftp, "fluid3d.zip")
    SendFile(ftp, "fluid3d/Raycast.glsl")
    SendFile(ftp, "fluid3d/Fluid.glsl")
    SendFile(ftp, "fluid3d/Fluid3d.cpp")
    SendFile(ftp, "fluid3d/Utility.cpp")

print colorama.Fore.WHITE + colorama.Back.RED
os.system('svn status .. -q')
print colorama.Fore.RESET + colorama.Back.RESET
print "DON'T FORGET TO UPDATE SVN "
