networkit_add_module(components
    BiconnectedComponents.cpp
    ConnectedComponents.cpp
    ConnectedComponentsImpl.cpp
    ComponentDecomposition.cpp
    DynConnectedComponents.cpp
    DynWeaklyConnectedComponents.cpp
    ParallelConnectedComponents.cpp
    RandomSpanningForest.cpp
    StronglyConnectedComponents.cpp
    WeaklyConnectedComponents.cpp
    )

networkit_module_link_modules(components
    auxiliary coarsening dynamics graph structures)

add_subdirectory(test)

