Antesconvert : automatic music sheet converter from MusicXML files to Antescofo language.


Antesconvert Features: 
----------------------

* automatic conversion from MusicXML files to Antescofo Language.
* selection of voices and staff for conversion
* selection of measure and measures interval for conversion.
* export to midicent Antescofo notes description 
* export to note names Antescofo notes description 



Usage :
--------

1/ Launch Antesconvert application,
2/ drag'n'drop a MusicXML music sheet exported from a software like Finale, Sybelius or MuseScore,
	or click on the "Load score" button,
	- if you want to convert notes to midicent notation : 
	   uncheck "Convert to midicents to note names" toggle button.
  - if you want to export just a few measures add an interval like "12-38"
	   in the measures text field, or just a number of measure like "67",
	- if you want to export just a specific voice or staff
	   uncheck the voices or staff you don't want to export
3/ Click on Convert
4/ Click on Save, then Choose a folder to store your converted Antescofo score.

That's it !



How to have better result while using Antesconvert with multiple staves :
-------------------------------------------------------------------------

When using multiple parts in your music sheet your can encounter several
problems, one way to solve them is :
Try merging multiple staves on only one staff containing all the score,
then export to MusicXML and convert.



Dependencies :
--------------

Antesconvert is built on OpenFrameworks, version 74 and libmusicxml 3.0 from Grame.
see https://code.google.com/p/libmusicxml/




Thomas Coffy
thomas.coffy@ircam.fr

http://repmus.ircam.fr/antescofo
