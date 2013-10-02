//Function which cues the manual recording. It's called by a function in WaveRecordPlay when that function detects
//when there is nothing in that index.
void recordFromButtons(int currentComb) {
  Serial.print("in recordFromButtons, RECORDING ");
  Serial.println(currentComb);
  trackRecord(currentComb, 'r');
}
