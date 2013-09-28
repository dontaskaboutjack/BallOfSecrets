/*
The code on this page is separate to keep it clear from the other files
It handles just the main new function code for the music ball
*/

//Function which cues the WaveRecordPlay code to search for a file ata particular location
void searchForFile(int currentComb) {
  Serial.println("made it to search for file"); 
  if (currentComb == 0) {return;} 
  else {  
    Serial.print("we're in searchForFile and currentComb is ");
    Serial.println(currentComb);
    trackPlay(currentComb);
  }
}

//Function which cues the manual recording. It's called by a function in WaveRecordPlay when that function detects
//when there is nothing in that index.
void recordFromButtons(int currentComb) {
  Serial.print("in recordFromButtons, RECORDING ");
  Serial.println(currentComb);
  trackRecord(currentComb, 'r');  
}
