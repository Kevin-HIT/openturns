#! /usr/bin/env python

from __future__ import print_function
from openturns import *

TESTPREAMBLE()

try:
    data = NumericalPoint()

    # DEBUG ; ici attente de la TUI
    palette = Description()
    labels = Description()

    data.add(5665855)
    labels.add("Candidate1")
    palette.add("blue")

    data.add(4804713)
    labels.add("Candidate2")
    palette.add("white")

    data.add(4610113)
    labels.add("Candidate3")
    palette.add("pink")

    data.add(1949170)
    labels.add("Candidate4")
    palette.add("yellow")

    data.add(1630045)
    labels.add("Candidate5")
    palette.add("red")

    data.add(1518528)
    labels.add("Candidate6")
    palette.add("ivory")

    data.add(1495724)
    labels.add("Candidate7")
    palette.add("green")

    data.add(1210562)
    labels.add("Candidate8")
    palette.add("orange")

    data.add(1204689)
    labels.add("Candidate9")
    palette.add("maroon")

    data.add(1113484)
    labels.add("Candidate10")
    palette.add("violet")

    data.add(3295588)
    labels.add("cumul candidates < 3.5% ")
    palette.add("purple")

    # Create an empty graph
    myGraph = Graph(
        "Presidentielle 2002 - 1er tour", "x1", "x2", True, "topright")

    # Create the first cloud
    myPie1 = Pie(data, labels, NumericalPoint(2), 1, palette)

    # Then, draw it
    myGraph.add(myPie1)
    myGraph.draw("Graph_Pie_OT", 640, 480)

   # Check that the correct files have been generated by computing their
   # checksum

except:
    import sys
    print("t_Pie_std.py", sys.exc_info()[0], sys.exc_info()[1])