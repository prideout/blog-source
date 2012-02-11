from xml.dom.minidom import parse
import codecs

def getText(nodelist):
    rc = ""
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc = rc + node.data
    return rc

f = codecs.open( "wordle.txt", "w", "utf-8" )

for i in xrange(0, 10):
    filename = "ch%02d.xml" % i
    tree = parse(filename)
    paras = tree.getElementsByTagName("para")
    for para in paras:
        f.write(getText(para.childNodes))

print "wordle.txt has been dumped"
