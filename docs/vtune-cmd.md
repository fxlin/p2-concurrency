# VTune command line results viewer

For those who are still struggling with MacOS... Do this from granger1/2. Quite primitive but may be useful. 

## Generating a Report

You can also use the vtune command with the -report option to generate a report. There are several ways to group data in a report, as follows:

To report time grouped by functions (in descending order) and print the results to stdout, or to a specific output text file, such as output.txt:

```
vtune -report hotspots -r r000hs
```

or

```
vtune -report hotspots -r r000hs -report-output output
```

To report time grouped by source lines, in descending order:

```
vtune -report hotspots -r r000hs -group-by source-line
```

To report time grouped by module:

```
vtune -report hotspots -r r000hs -group-by module
```

### Reference: 

https://www.nas.nasa.gov/hecc/support/kb/finding-hotspots-in-your-code-with-the-intel-vtune-command-line-interface_506.html