BallOfSecrets (Arduino)
=============

Required libraries:

* https://code.google.com/p/sdfatlib/
* https://code.google.com/p/waverp/ (Modified)
  * (line 429) `if (!sdCard->erase(sdStartBlock, sdEndBlock)) return false;` has been commented out

Debouncing code inspiration:

http://www.adafruit.com/blog/2009/10/20/example-code-for-multi-button-checker-with-debouncing/
