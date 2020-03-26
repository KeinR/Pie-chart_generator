## Pichart generator

It's what it sounds like. Includes titles, resizability, and a key, all controlled via the config file.

Usage:

`./chartgen.exe <config files>`

Where `<config files>` can be more than one config file, each successive one effectively being appended to the last.

Config file syntax:

`<value (decimal or integer)> #<hex color> <name>` the main attraction, this is how you add a new value to the pi-chart. The first parameter is the number for the data group, the second is the name that will be used to refer to it on the key (spaces permitted), and the third specifies a hex color to use for it.

The rest here are for literal configuration, all of which have defaults, and are case insensitive:

Terms to know:
* `exString`: Chars will be captured from after the *LAST* space after the command,
ex "@title  asda" will result in a title "asda"
* `incString`: Chars wil be captured from after the *FIRST* space after the command, 
ex "@suffix  asda" will result in a suffix " asda"
* `integer`: You probally know this one, but just to be clear: only chars 0-9, keep it less than 2147483647 (0x7FFFFFFF) and above 0 to prevent seg faults, although that shouldn't be a problem for the average person...

`@title <exString>` sets the title.
Default: -blank-

`@out <exString>` sets where to save the resultant png; bad filenames will result in failure.
Default: "./piechart.jpg"

`@suffix <incString>` sets the suffix to use. If `@percent` is set and this isn't, will default to `%`.
Default: -blank-

`@prefix <incString>` sets the prefix to use.
Default: -blank-

`@radius <integer>` sets the chart's radius.
Default: 100

`@margin <integer>` the space between the chart and the title, key, etc.
Default: 10

`@titlesize <integer>` sets the title's font size.
Default: 10

`@descwrap <integer>` sets the wrapping width for the sidebar (aka the key).
Default: 50

`@descsize <integer>` sets the font size for chars in the sidebar (aka the key).
Default: 10

`@percent` toggles displaying of percent data instead of value data to **TRUE**
Default: TRUE

`@value` toggles displaying of percent data instead of value data to **FALSE**, meaning that the raw values will be displayed.
Default: FALSE; that is, percent values will be displayed.

*Do note that all the defaults can be changed in the source file*

### Dependencies

This of course wouldn't have been possible without the single-file header C librarys, [stb_image_write.h](https://github.com/nothings/stb/blob/master/stb_image_write.h) and [stb_truetype.h](https://github.com/nothings/stb/blob/master/stb_truetype.h) by [Sean T. Barrett](https://github.com/nothings) located at the [stb repositry](https://github.com/nothings/stb)
