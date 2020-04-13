je nach erzeugetr payload muss der decoder angepasst werden, um die korrekten Werte weiterverarbeiten zu können

function Decoder(bytes, port) {

  var test =   (bytes[0] + (bytes[1] << 8) + (bytes[2] << 16));
  var weight = (bytes[3] + (bytes[4] << 8) + (bytes[5] << 16)) / 100;
  var batt  =  (bytes[6] + (bytes[7] << 8))/100;
  var temp1  = (bytes[8] + (bytes[9] << 8))/100;

  //return decoded;
  return {
    field1: test,
    field2: weight,
    field3: batt,
    field4: temp1,
  }
  
}

