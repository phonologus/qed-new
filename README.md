The _qed_ editor for Unix
=========================

This is a fresh port of the venerable _qed_ editor for Unix, first
written by Tom Duff, Rob Pike, Hugh Redelmeier and David Tilbrook at
the University of Toronto in the late 1970's.

_Qed_ is described completely in the `qed(1)` manpage. It is a line
editor, in the tradition of _ed_, with multi-file editing, and
a powerful, if somewhat cryptic, programming language.

The version here has been updated to compile on a modern ANSI/POSIX
system, and has had the ability to work with UTF-8 encoded text added.

This port supersedes, and renders obsolete, my previous port [here][Q].

[Q]: https://github.com/phonologus/QED

Building
========

Building and installing _qed_ should be as easy as:

    make clean && make && make install

Installation is into a self-contained directory `$HOME/.qed`
To uninstall, `make uninstall`, or `rm -rf $HOME/.qed`.

The line `. $HOME/.qed/env` should be added to your `.profile`
or `.bash_profile`.

The _qed_ binary is installed into `.qed/bin`, and the manpage
into `.qed/man/man1`. The startup file, and the library
programs are in `.qed/lib`. The script `.qed/env`
ensures `.qed/bin` is in PATH, and it sets up the QEDFILE
and QEDLIB envrionment variables. Make changes to the `env`
script to point QEDFILE and QEDLIB elsewhere if you need.

New Features
============

UTF-8
-----

This verison of _qed_ has two notions of what a "character" is, controlled
by a new option `a`. If `a` is _unset_ (which is the dafault), then
_qed_ assumes that a character is a UTF-8 encoded Unicode codepoint, and
that the text being processed is valid UTF-8 encoded text. In this mode,
_Qed_ will throw a `?U` error if it is asked to perform a character-oriented
action on invalid UTF-8. It will still save and load invalid UTF-8, but will
issue a warning notification `!U`.

If `a` is _set_, then _qed_ assumes that a character is an 8-bit byte.
In this case, _qed_ can process any single-byte character encoding, such
as Latin-1. Since _qed_ can edit any single byte in this mode,
it can be used to fix broken UTF-8 on a byte-by-byte basis.

The `a` option can be set from the commandline, with option `-a`, and/or
it can be set or reset at any time by issuing the `oas` or `oar` commands,
as with _qed_'s other options.

**Beware!** Since the last used regular expression is saved in its
compiled form for possible
re-use, unexpected results can occur if the regular expression was first
compiled in one mode, and is reinvoked when in the other mode!

**Beware!** The rendering of UTF-8 encoded text is entirely under the control
of the terminal running _qed_. _Qed_ itself has no understanding of
Unicode semantics, such as character width or directionality. This
means that some of the visually-orientated features of _qed_
(mainly the `x` command's visual editing mode, and several of the
programs in the library, such as `paren.q`) are likely to have unexpected
outputs with text that is anything more exotic than single-width,
non-combining characters.

Other new features
------------------

This _qed_ has a new special character `\0`, followed by up to
three octal digits, which allows the insertion of any byte value.
The motivation for this is largely to be able to fix broken UTF-8
on a byte-by-byte basis in conjunction with setting the `a` option,
although other more exotic transformations of UTF-8 text are
certainly possible.

This _qed_ has a new register command `]` which complements
the existing command `[`. Whereas `[` puts the index of the
_first_ matched character into the _Count_ register, `]`
puts the index of the _last_ matched character into the
_Count_ register. For example:

    za:kungfoo
    za[/foo/
    zCp
        4    "match starts at index 4
    za]/foo/
    zCp
        6    "match ends at index 6

Sources
=======

This version of _qed_ is derived from the Research Unix Version 8
sources available from the Unix Archive [here][1], and located
in [`/usr/local/src/cmd/qed`][2].

[1]: https://www.tuhs.org/Archive/Distributions/Research/Dan_Cross_v8/
[2]: https://www.tuhs.org/cgi-bin/utree.pl?file=V8/usr/src/cmd/qed

The manpage is derived from the Research Unix Version 10 manpage,
also available from the Unix Archive [here][3], and located
in [`/man/manb/qed.1`][4].

[3]: https://www.tuhs.org/Archive/Distributions/Research/Norman_v10/
[4]: https://www.tuhs.org/cgi-bin/utree.pl?file=V10/man/manb/qed.1

The library of _qed_ programs is derived from Rob Pike's `q`
directory, found in Arnold Robbins' _qed-archive_ [here][5].
The library seems to be an updated version of a similar library that
was released with an earlier version of the _qed_ sourcecode
at the 1980 Usenix Delaware conference, and available at
the Unix Archive [here][6], in directory `boulder/caltech`
in the archive `usenix_80_delaware.tar.gz`.

[5]: https://github.com/arnoldrobbins/qed-archive/tree/master/unix-1992/
[6]: https://www.tuhs.org/Archive/Applications/Shoppa_Tapes/

Authors
=======

The original sourcecode for _qed_ was written by Tom Duff, Rob Pike,
Hugh Redelmeier and David Tilbrook at the University of Toronto
in the late 1970's, based on the U. of T.'s version of the Unix
Version 6 editor _ed_.

In 2024, Sean Jensen reformatted the authors' original
sourcecode to be ANSI-compliant C, and made changes to (i) make
it compile on an up-to-date ANSI/POSIX system; and (ii) to add new
capabilities for processing UTF-8 encoded text.

He also added some text to the manpage describing these
new capabilities.

The source files `alloc.c`, `u.c` and `utf.[ch]` were written entirely
by Sean Jensen.

