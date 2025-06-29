# How to contribute to HiGHS

Welcome! This document explains some of the ways you can contribute to HiGHS.

## Code of Conduct

This project and everyone participating in it is governed by the [HiGHS Code of Conduct](https://github.com/ERGO-Code/HiGHS/blob/master/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Contact the HiGHS team

Contact [Julian](https://github.com/jajhall) (General issues and solvers), [Ivet](https://github.com/galabovaa)  (Build and interfaces)

## Improve the documentation

The top level [documentation](https://ergo-code.github.io/HiGHS/) is created using [Documenter](https://documenter.juliadocs.org/stable/), with the files held on the [HiGHS repository](https://github.com/ERGO-Code/HiGHS/tree/master/docs). If your change is small (like fixing typos, or one or two sentence corrections), the easiest way to do this is to fork the branch and create a pull request. (See *Contribute code to HiGHS* below for more on this.) If your change is larger, or touches multiple files, please raise an issue describing what you want to do.

## Raise an issue

You can raise an [issue](https://github.com/ERGO-Code/HiGHS/issues) with HiGHS in the normal way via [GitHub](https://docs.github.com/en/issues/tracking-your-work-with-issues/creating-an-issue).

## Contribute code to HiGHS

HiGHS is open source for distribution rather than contribution. This applies particularly to the core C++ code of the solvers. However, there is definitely scope for external contribution to interfaces and documentation. If you want to contribute in this way, please open an issue before making a pull request, since pull requests to the HiGHS solvers will not normally be accepted. 

Please note that any pull requests to HiGHS must be made to the `latest` branch, so that full testing can take place befor updating `master`. This ensures that `master` is always good to be downloaded by users.

Note that, under the terms of the MIT license, by contributing to HiGHS you assign away your rights to the content of your contribution.
