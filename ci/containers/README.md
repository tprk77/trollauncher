# Docker Containers #

## Ubuntu Containers ##

These are easy. Just use Docker like normal.

## Windows Containers ##

These are a pain, and I ran into a few issues getting them to work.

First, I had some weird DNS issues on my machine. To get anything to resolve, I
had to use the `--dns` argument or change the daemon settings. Not a big deal
but annoying. FYI, I did not have this issue when I tried it on an Azure VM, so
it might just be that particular laptop.

Second, I tried using `windows/nanoserver`, but it seemed that MSYS2 didn't
really work on it. Fortunately, it did work on `windows/servercore`. The
downside is that it's a 5 GB container! So we better make sure we are building
off of [GitHub's cached image][gh_windows_img], which is currently
`windows/servercore:ltsc2019`.

Third, `docker build` would hang and never complete. It turns out it's [getting
stuck trying to export the layers][docker_hcsshim_issue]. I found a workaround
that's [a massive hack][docker_msys2_issue], but at least it works. Basically,
you need to run some random `docker-ci-zap` to unblock the build process.

<!-- Links -->

[gh_windows_img]: https://github.com/actions/virtual-environments/blob/main/images/win/Windows2019-Readme.md
[docker_hcsshim_issue]: https://github.com/microsoft/hcsshim/issues/696
[docker_msys2_issue]: https://github.com/msys2/MSYS2-packages/issues/2305
