networkit_add_module(graph
    EdgeIterators.cpp
    Graph.cpp
    GraphBuilder.cpp
    GraphTools.cpp
    KruskalMSF.cpp
    PrimMSF.cpp
    RandomMaximumSpanningForest.cpp
    SpanningForest.cpp
    TopologicalSort.cpp
    UnionMaximumSpanningForest.cpp
    )

networkit_module_link_modules(graph
    auxiliary structures)

add_subdirectory(test)

