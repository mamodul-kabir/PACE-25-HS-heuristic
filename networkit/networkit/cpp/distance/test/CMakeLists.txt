networkit_add_test(distance APSPGTest
    auxiliary dyn_distance generators io)
networkit_add_test(distance CommuteTimeDistanceGTest
    centrality graph io numerics)
networkit_add_test(distance DistanceGTest
    dyn_distance generators io)
networkit_add_test(distance GraphDistanceGTest
    io)
networkit_add_test(distance SSSPGTest
    auxiliary io)
networkit_add_test(dyn_distance DynSSSPGTest
        auxiliary generators graph io)
networkit_add_test(distance FloydWarshallGTest
        graph)
