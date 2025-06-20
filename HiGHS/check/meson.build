# -------------------------- Configuration
test_data_conf = configuration_data()
test_data_conf.set_quoted('HIGHS_DIR',
                          meson.project_source_root())
configure_file(
  input: 'HCheckConfig.h.meson.in',
  output: 'HCheckConfig.h',
  configuration: test_data_conf,
)

test_array = [
  ['test_highsversion', 'TestHighsVersion.cpp'],
  ['test_alienbasis', 'TestAlienBasis.cpp'],
  ['test_dualize', 'TestDualize.cpp'],
  ['test_checksolution', 'TestCheckSolution.cpp'],
  ['test_callbacks', 'TestCallbacks.cpp'],
  ['test_ekk', 'TestEkk.cpp'],
  ['test_factor', 'TestFactor.cpp'],
  ['test_main', 'TestMain.cpp'],
  ['test_names', 'TestNames.cpp'],
  ['test_options', 'TestOptions.cpp'],
  ['test_io', 'TestIO.cpp'],
  ['test_sort', 'TestSort.cpp'],
  ['test_setup', 'TestSetup.cpp'],
  ['test_filereader', 'TestFilereader.cpp'],
  ['test_highsgfksolve', 'TestHighsGFkSolve.cpp'],
  ['test_info', 'TestInfo.cpp'],
  ['test_basis', 'TestBasis.cpp'],
  ['test_basissolves', 'TestBasisSolves.cpp'],
  ['test_crossover', 'TestCrossover.cpp'],
  ['test_highshash', 'TestHighsHash.cpp'],
  ['test_highsintegers', 'TestHighsIntegers.cpp'],
  ['test_highsparallel', 'TestHighsParallel.cpp'],
  ['test_highsrbtree', 'TestHighsRbTree.cpp'],
  ['test_highshessian', 'TestHighsHessian.cpp'],
  ['test_highsmodel', 'TestHighsModel.cpp'],
  ['test_highssparsematrix', 'TestHighsSparseMatrix.cpp'],
  ['test_hset', 'TestHSet.cpp'],
  ['test_icrash', 'TestICrash.cpp'],
  ['test_logging', 'TestLogging.cpp'],
  ['test_lpfileformat', 'TestLPFileFormat.cpp'],
  ['test_ipx', 'TestIpx.cpp'],
  ['test_presolve', 'TestPresolve.cpp'],
  ['test_qpsolver', 'TestQpSolver.cpp'],
  ['test_rays', 'TestRays.cpp'],
  ['test_ranging', 'TestRanging.cpp'],
  ['test_semivariables', 'TestSemiVariables.cpp'],
  ['test_throw', 'TestThrow.cpp'],
  ['test_speciallps', 'TestSpecialLps.cpp'],
  ['test_lpsolvers', 'TestLpSolvers.cpp'],
  ['test_mipsolver', 'TestMipSolver.cpp'],
  ['test_lpvalidation', 'TestLpValidation.cpp'],
  ['test_lpmodification', 'TestLpModification.cpp'],
  ['test_lporientation', 'TestLpOrientation.cpp'],
]
foreach test : test_array
  test(test.get(0),
       executable(test.get(0),
          sources : ['TestMain.cpp',
                     'Avgas.cpp',
                     test.get(1),
                     highs_conf_file],
          dependencies : _deps,
          link_with : highslib,
          cpp_args : _args,
          include_directories: _incdirs,
                 ),
       timeout: 300,
      )
endforeach


test('test_capi',
     executable('capi_unit_tests',
                sources: ['TestCAPI.c', highs_conf_file],
                include_directories: _incdirs,
                link_with : highslib,
               )
    )
