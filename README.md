## Pichart generator

It's what it sounds like. Includes titles, resizability, and a key, all controlled via the config file.

Config file syntax:

`<number> <name> <hex color value>` the main attraction, this is how you add a new value to the pi-chart. The first parameter is the number for the data group, the second is the name that will be used to refer to it on the key (spaces permitted), and the third specifies a hex color to use for it.

The rest here are for literal configuration, all of which have defaults, and are case insensitive:

`@radius <number>` specifies the chart's radius.

`@margin <number>` the space between the chart and the title, key, etc.

`@outputpath <filepath>` specifies where to save the resultant png.

`@title <string>` specifies the title

`@titlesize <number>` specifies the title's font size
