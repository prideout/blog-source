#!/usr/bin/python

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

def SendFile(ftp, filename):
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

postid = 22
filename = 'Article.txt'
createBackup = True
spellCheck = True

blog = pyblog.WordPress('http://prideout.net/blog/xmlrpc.php', 'admin', passwd)

# If this can't find the post, it'll throw an exception with a good error message.
# Since it goes uncaught, it aborts the program.  Which is fine.
post = blog.get_post(postid)

print "Found post %d with the following keys:" % postid
print '\n'.join(post.keys())

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

if createBackup:
    backup = codecs.open(filename + '.bak', 'wt', 'utf-8')
    backup.write(post['description'])

post['description'] = contents
publish = False
blog.edit_post(postid, post, publish)
print "Updated 'description' successfully.\n Attempting FTP..."

DifferentPassword = False
if DifferentPassword:
    print "FTP Login "
    passwd = getpass.getpass()

ftp = FTP('ftp.prideout.net')
ftp.login('prideout', passwd)
ftp.retrlines('LIST')
print
ftp.cwd('prideout.net/blog')
ftp.retrlines('LIST')
print

try:
    ftp.mkd('aacel')
except:
    pass

ftp.cwd('aacel')

ftp.retrlines('LIST')

try:
    shutil.rmtree('aacel')
except:
    pass

os.system('svn export .. aacel')

try:
    shutil.rmtree('aacel/deploy')
except:
    pass

z = zipfile.ZipFile('aacel.zip', 'w')
for name in glob.glob("aacel/*"):
    if not os.path.isdir(name):
        z.write(name, os.path.basename(name), zipfile.ZIP_DEFLATED)
for name in glob.glob("aacel/lib/*"):
    z.write(name, 'lib/' + os.path.basename(name), zipfile.ZIP_DEFLATED)
z.close()

if True:
    t = tarfile.TarFile('aacel.tar.gz', 'w')
    t.close()

    t = tarfile.TarFile.open('aacel.tar.gz', 'w:gz')
    for name in glob.glob("aacel/*"):
        if not os.path.isdir(name):
            t.add(name, os.path.basename(name), zipfile.ZIP_DEFLATED)
    for name in glob.glob("aacel/lib/*"):
        t.add(name, 'lib/' + os.path.basename(name), zipfile.ZIP_DEFLATED)
    t.close()

SendFile(ftp, "aacel.zip")
SendFile(ftp, "aacel.tar.gz")
SendFile(ftp, "aacel/Trefoil.cpp")

print colorama.Fore.WHITE + colorama.Back.RED
os.system('svn status .. -q')
print colorama.Fore.RESET + colorama.Back.RESET
print "DON'T FORGET TO UPDATE SVN "
