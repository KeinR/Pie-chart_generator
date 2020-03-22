## Pichart generator

It's what it sounds like. Includes titles, resizability, and a key, all controlled via the config file specified on line 38 in net/keinr/pichart/Main.java. The default path is `resources/config.txt`.

Config file syntax:

`<number> <name> <hex color value>` the main attraction, this is how you add a new value to the pi-chart. The second and third parameters are *optional*, however I recommend you fill out at least the `<name>` field as well. The first parameter is the number for the data group, the second is the name that will be used to refer to it on the key (spaces ARE allowed\*), and the third specifies a hex color to use for it (if you don't know what that is, look up, "Hex color picker". The value should look something like this -> #621c77; a jumble of numbers & letters, with a hash mark)

The rest here are for literal configuration:

`@radius <number>` specifies the chart's radius. This is primarily to get a high-quality chart.

`@margin <number>` the space between the chart and the wall, title, key, etc.

`@outputpath <filepath>` specifies where to save the completed file, WITH THE NAME. For example, the default is `output/output.png`

`@title <string>` specifies the title

`@titlesize <number>` specifies the title's font size

