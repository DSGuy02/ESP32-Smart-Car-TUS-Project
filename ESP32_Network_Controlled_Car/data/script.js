// Motor control functions
function moveForward() {
  fetch('/forward');
}

function moveLeft() {
  fetch('/left');
}

function stopRobot() {
  fetch('/stop');
}

function moveRight() {
  fetch('/right');
}

function moveReverse() {
  fetch('/reverse');
}

function updateMotorSpeed(pos) {
  document.getElementById('motorSpeed').innerHTML = pos;
  fetch(`/speed?value=${pos}`);
}

function sendRadioMessage() {
  const msg = document.getElementById('radioInput').value;
  if (msg.trim()) {
    fetch(`/radio?msg=${encodeURIComponent(msg)}`);
    document.getElementById('radioInput').value = '';
  }
}

function updateTelemetry() {
  fetch('/telemetry')
    .then(response => response.json())
    .then(data => {
      document.getElementById('distance').innerHTML = data.distance;
      document.getElementById('wifiSignal').innerHTML = data.wifi;
      document.getElementById('radioStatus').innerHTML = data.radio;
      document.getElementById('gpsLat').innerHTML = data.latitude;
      document.getElementById('gpsLon').innerHTML = data.longitude;
      document.getElementById('gpsSats').innerHTML = data.satellites;
      document.getElementById('gpsStatus').innerHTML = data.gpsStatus;
    })
    .catch(error => {
      console.error('Telemetry update failed:', error);
    });
}

// Keyboard controls
document.addEventListener('keydown', function(event) {
  switch(event.key) {
    case 'ArrowUp':
    case 'w':
    case 'W':
      moveForward();
      break;
    case 'ArrowDown':
    case 's':
    case 'S':
      moveReverse();
      break;
    case 'ArrowLeft':
    case 'a':
    case 'A':
      moveLeft();
      break;
    case 'ArrowRight':
    case 'd':
    case 'D':
      moveRight();
      break;
    case ' ':
      event.preventDefault();
      stopRobot();
      break;
  }
});

// Radio input enter key
document.getElementById('radioInput').addEventListener('keypress', function(event) {
  if (event.key === 'Enter') {
    sendRadioMessage();
  }
});

// Start telemetry updates
setInterval(updateTelemetry, 1000);

// Initial telemetry load
document.addEventListener('DOMContentLoaded', function() {
  updateTelemetry();
});