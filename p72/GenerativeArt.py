import random
import sys
from euclid import *
from elementtree.ElementTree import ElementTree as etree
from elementtree.ElementTree import fromstring

Library = {}

Library["Tree"] = """
<rules max_depth="200">
    <rule name="entry">
        <call transforms="ry 180" rule="spiral"/>
    </rule>
    <rule name="spiral" weight="100">
        <instance shape="tubey"/>
        <call transforms="ty 0.4 rx 1 sa 0.995" rule="spiral"/>
    </rule>
    <rule name="spiral" weight="100">
        <instance shape="tubey"/>
        <call transforms="ty 0.4 rx 1 ry 1 sa 0.995" rule="spiral"/>
    </rule>
    <rule name="spiral" weight="100">
        <instance shape="tubey"/>
        <call transforms="ty 0.4 rx 1 rz -1 sa 0.995" rule="spiral"/>
    </rule>
    <rule name="spiral" weight="6">
        <call transforms="rx 15" rule="spiral"/>
        <call transforms="ry 180" rule="spiral"/>
    </rule>
</rules>"""

Library["Octopod"] = """
<rules max_depth="20">
    <rule name="entry">
        <call count="300" transforms="ry 3.6" rule="arm"/>
    </rule>
    <rule name="arm">
        <call transforms="sa 0.9 rz 6 tx 1" rule="arm"/>
        <instance transforms="s 1 0.2 0.5" shape="box"/>
    </rule>
    <rule name="arm">
        <call transforms="sa 0.9 rz -6 tx 1" rule="arm"/>
        <instance transforms="s 1 0.2 0.5" shape="box"/>
    </rule>
</rules>
"""

Library["Nouveau"] = """
<rules max_depth="2000">
    <rule name="entry">
        <call count="16" transforms="rz 20" rule="hbox"/>
    </rule>
    <rule name="hbox"><call rule="r"/></rule>
    <rule name="r"><call rule="forward"/></rule>
    <rule name="r"><call rule="turn"/></rule>
    <rule name="r"><call rule="turn2"/></rule>
    <rule name="r"><call rule="turn4"/></rule>
    <rule name="r"><call rule="turn3"/></rule>
    <rule name="forward" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="rz 2 tx 0.1 sa 0.996" rule="forward"/>
    </rule>
    <rule name="turn" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="rz 2 tx 0.1 sa 0.996" rule="turn"/>
    </rule>
    <rule name="turn2" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="rz -2 tx 0.1 sa 0.996" rule="turn2"/>
    </rule>
    <rule name="turn3" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="ry -2 tx 0.1 sa 0.996" rule="turn3"/>
    </rule>
    <rule name="turn4" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="ry -2 tx 0.1 sa 0.996" rule="turn4"/>
    </rule>
    <rule name="turn5" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="rx -2 tx 0.1 sa 0.996" rule="turn5"/>
    </rule>
    <rule name="turn6" max_depth="90" successor="r">
        <call rule="dbox"/>
        <call transforms="rx -2 tx 0.1 sa 0.996" rule="turn6"/>
    </rule>
    <rule name="dbox">
        <instance transforms="s 0.55 2.0 1.25" shape="boxy"/>
    </rule>
</rules>
"""

def Evaluate(rules, seed = 0):
    """
    Takes an XML string (see the Library) and return a list of shapes.
    Each shape is a 2-tuple: (shape name, transform matrix).
    """

    def radians(d):
        return float(d * 3.141 / 180.0)
    
    def pick_rule(tree, name):
    
        rules = tree.findall("rule")
        elements = []
        for r in rules:
            if r.get("name") == name:
                elements.append(r)

        if len(elements) == 0:
            print "Error, no rules found with name '%s'" % name
            quit()
    
        sum, tuples = 0, []
        for e in elements:
            weight = int(e.get("weight", 1))
            sum = sum + weight
            tuples.append((e, weight))
        n = random.randint(0, sum - 1)
        for (item, weight) in tuples:
            if n < weight:
                break
            n = n - weight
        return item

    def parse_xform(xform_string):
        matrix = Matrix4.new_identity()
        tokens = xform_string.split(' ')
        t = 0
        while t < len(tokens) - 1:
            command, t = tokens[t], t + 1

            # Translation
            if command == 'tx':
                x, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_translate(x, 0, 0)
            elif command == 'ty':
                y, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_translate(0, y, 0)
            elif command == 'tz':
                z, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_translate(0, 0, z)
            elif command == 't':
                x, t = float(tokens[t]), t + 1
                y, t = float(tokens[t]), t + 1
                z, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_translate(x, y, z)

            # Rotation
            elif command == 'rx':
                theta, t = radians(float(tokens[t])), t + 1
                matrix *= Matrix4.new_rotatex(theta)
            elif command == 'ry':
                theta, t = radians(float(tokens[t])), t + 1
                matrix *= Matrix4.new_rotatey(theta)
            elif command == 'rz':
                theta, t = radians(float(tokens[t])), t + 1
                matrix *= Matrix4.new_rotatez(theta)

            # Scale
            elif command == 'sx':
                x, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_scale(x, 1, 1)
            elif command == 'sy':
                y, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_scale(1, y, 1)
            elif command == 'sz':
                z, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_scale(1, 1, z)
            elif command == 'sa':
                v, t = float(tokens[t]), t + 1
                x, y, z = v, v, v
                matrix *= Matrix4.new_scale(x, y, z)
            elif command == 's':
                x, t = float(tokens[t]), t + 1
                y, t = float(tokens[t]), t + 1
                z, t = float(tokens[t]), t + 1
                matrix *= Matrix4.new_scale(x, y, z)

            else:
                print "unrecognized transformation: '%s' at position %d in '%s'" % (command, t, xform_string)
                quit()

        return matrix

    random.seed(seed)
    tree = fromstring(rules)
    entry = pick_rule(tree, "entry")
    shapes = []
    stack = []
    stack.append((entry, 0, Matrix4.new_identity()))
    max_depth = int(tree.get("max_depth"))

    progressCount = 0
    print "Evaluating Lindenmayer system",
    while len(stack) > 0:

        if len(shapes) > progressCount + 1000:
            print ".",
            progressCount = len(shapes)
    
        rule, depth, matrix = stack.pop()

        local_max_depth = max_depth
        if "max_depth" in rule.attrib:
            local_max_depth = int(rule.get("max_depth"))

        if len(stack) >= max_depth:
            continue

        if depth >= local_max_depth:
            if "successor" in rule.attrib:
                successor = rule.get("successor")
                rule = pick_rule(tree, successor)
                stack.append((rule, 0, matrix))
            continue
    
        for statement in rule:
            xform = parse_xform(statement.get("transforms", ""))
            count = int(statement.get("count", 1))
            for n in xrange(count):
                matrix *= xform
                if statement.tag == "call":
                    rule = pick_rule(tree, statement.get("rule"))
                    cloned_matrix = matrix.copy()
                    stack.append((rule, depth + 1, cloned_matrix))
                elif statement.tag == "instance":
                    name = statement.get("shape")
                    shape = (name, matrix)
                    shapes.append(shape)
                else:
                    print "malformed xml"
                    quit()

    print "\nGenerated %d shapes." % len(shapes)
    return shapes
