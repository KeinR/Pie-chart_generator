## Pichart generator

It's what it sounds like. Includes titles, resizability, and a key, all controlled via the config file.

Config file syntax:

`<number> <name> <hex color value>` the main attraction, this is how you add a new value to the pi-chart. The first parameter is the number for the data group, the second is the name that will be used to refer to it on the key (spaces permitted), and the third specifies a hex color to use for it.

The rest here are for literal configuration, all of which have defaults, and are case insensitive:

Terms to know:
* `exString`: Chars will be captured from after the *LAST* space after the command,
ex "@title  asda" will result in a title "asda"
* `incString`: Chars wil be captured from after the *FIRST* space after the command, 
ex "@suffix  asda" will result in a suffix " asda"

`@title <exString>` sets the title; default: blank

`@out <exString>` sets where to save the resultant png; bad filenames will result in failure. Default:

`@suffix <incString>` sets the suffix to use 

`@prefix <incString>` 

`@radius <number>` specifies the chart's radius.

`@margin <number>` the space between the chart and the title, key, etc.

`@titlesize <number>` specifies the title's font size

`@titlesize <number>` specifies the title's font size
