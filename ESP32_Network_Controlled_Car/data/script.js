// Global state
let isConnected = false;
let currentDirection = null;
let messageHistory = [];

// Motor control functions with visual feedback
function moveForward() {
  executeCommand('/forward', 'forward');
}

function moveLeft() {
  executeCommand('/left', 'left');
}

function stopRobot() {
  executeCommand('/stop', 'stop');
}

function moveRight() {
  executeCommand('/right', 'right');
}

function moveReverse() {
  executeCommand('/reverse', 'reverse');
}

function startBuzzer() {
  fetch('/buzzer?state=on')
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        document.getElementById('buzzerBtn').classList.add('active');
      }
    })
    .catch(error => {
      console.error('Buzzer start failed:', error);
      updateConnectionStatus(false);
    });
}

function stopBuzzer() {
  fetch('/buzzer?state=off')
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        document.getElementById('buzzerBtn').classList.remove('active');
      }
    })
    .catch(error => {
      console.error('Buzzer stop failed:', error);
      updateConnectionStatus(false);
    });
}

function toggleLED1() {
  fetch('/led1')
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        const btn = document.getElementById('led1Btn');
        btn.classList.toggle('active');
        // Update icon based on state
        const icon = btn.querySelector('.btn-icon');
        icon.textContent = btn.classList.contains('active') ? '●' : '○';
        icon.style.color = btn.classList.contains('active') ? '#ff4757' : '';
      }
    })
    .catch(error => {
      console.error('LED1 toggle failed:', error);
      updateConnectionStatus(false);
    });
}

function toggleLED2() {
  fetch('/led2')
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        const btn = document.getElementById('led2Btn');
        btn.classList.toggle('active');
        // Update icon based on state
        const icon = btn.querySelector('.btn-icon');
        icon.textContent = btn.classList.contains('active') ? '●' : '○';
        icon.style.color = btn.classList.contains('active') ? '#00d4ff' : '';
      }
    })
    .catch(error => {
      console.error('LED2 toggle failed:', error);
      updateConnectionStatus(false);
    });
}

function executeCommand(endpoint, direction) {
  fetch(endpoint)
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        highlightActiveButton(direction);
      } else {
        updateConnectionStatus(false);
      }
    })
    .catch(error => {
      console.error('Command failed:', error);
      updateConnectionStatus(false);
    });
}

function highlightActiveButton(direction) {
  // Remove active class from all buttons
  document.querySelectorAll('.control-btn').forEach(btn => {
    btn.classList.remove('active');
  });
  
  // Add active class to current button
  if (direction !== 'stop') {
    const btn = document.querySelector(`[data-direction="${direction}"]`);
    if (btn) {
      btn.classList.add('active');
      setTimeout(() => btn.classList.remove('active'), 200);
    }
  }
  
  currentDirection = direction;
}

function updateMotorSpeed(pos) {
  document.getElementById('motorSpeed').innerHTML = pos + '%';
  fetch(`/speed?value=${pos}`)
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        // Add visual feedback for speed change
        const speedValue = document.getElementById('motorSpeed');
        speedValue.style.transform = 'scale(1.2)';
        setTimeout(() => {
          speedValue.style.transform = 'scale(1)';
        }, 200);
      }
    })
    .catch(error => {
      console.error('Speed update failed:', error);
      updateConnectionStatus(false);
    });
}

function addSystemLog(message, type = 'received') {
  addMessageToHistory(message, type);
}

function addMessageToHistory(message, type) {
  const history = document.getElementById('messageHistory');
  const time = new Date().toLocaleTimeString('en-US', { 
    hour12: false, 
    hour: '2-digit', 
    minute: '2-digit' 
  });
  
  const messageDiv = document.createElement('div');
  messageDiv.className = `message ${type}`;
  messageDiv.innerHTML = `
    <span class="message-time">${time}</span>
    <span class="message-text">${message}</span>
  `;
  
  history.appendChild(messageDiv);
  history.scrollTop = history.scrollHeight;
  
  // Keep only last 50 messages
  const messages = history.querySelectorAll('.message');
  if (messages.length > 50) {
    messages[0].remove();
  }
}

function updateTelemetry() {
  fetch('/telemetry')
    .then(response => response.json())
    .then(data => {
      // Update telemetry values with animations
      updateValueWithAnimation('distance', data.distance);
      updateValueWithAnimation('wifiSignal', data.wifi);
      updateValueWithAnimation('gpsLat', data.latitude);
      updateValueWithAnimation('gpsLon', data.longitude);
      updateValueWithAnimation('gpsSats', data.satellites);
      
      // Update obstacle detection status
      updateObstacleStatus(data);
      
      // Update system status
      updateSystemStatus(data.motorStatus);
      
      // Update GPS status
      updateGPSStatus(data.gpsStatus, parseInt(data.satellites));
      
      // Update WiFi signal bars
      updateSignalBars(parseInt(data.wifi));
      
      // Update satellite visualization
      updateSatelliteVisual(parseInt(data.satellites));
      
      updateConnectionStatus(true);
    })
    .catch(error => {
      console.error('Telemetry update failed:', error);
      updateConnectionStatus(false);
    });
}

function updateValueWithAnimation(elementId, newValue) {
  const element = document.getElementById(elementId);
  if (element && element.textContent !== newValue) {
    element.style.transform = 'scale(1.1)';
    element.style.color = 'var(--accent-green)';
    element.textContent = newValue;
    
    setTimeout(() => {
      element.style.transform = 'scale(1)';
      element.style.color = '';
    }, 300);
  }
}

function updateConnectionStatus(connected) {
  isConnected = connected;
  const statusDot = document.querySelector('.connection-status .status-dot');
  const statusText = document.querySelector('.connection-status .status-text');
  
  if (connected) {
    statusDot.classList.add('online');
    statusText.textContent = 'Online';
  } else {
    statusDot.classList.remove('online');
    statusText.textContent = 'Offline';
  }
}

function updateGPSStatus(status, satellites) {
  const statusIndicator = document.getElementById('gpsStatusIndicator');
  const statusText = document.getElementById('gpsStatus');
  
  if (statusIndicator && statusText) {
    statusText.textContent = status;
    
    if (status === 'Fix' && satellites > 0) {
      statusIndicator.classList.add('connected');
    } else {
      statusIndicator.classList.remove('connected');
    }
  }
}

function updateSignalBars(wifiSignal) {
  const bars = document.querySelectorAll('#wifiSignalBars .bar');
  const strength = Math.max(0, Math.min(4, Math.floor((wifiSignal + 100) / 25) + 1));
  
  bars.forEach((bar, index) => {
    if (index < strength) {
      bar.classList.add('active');
    } else {
      bar.classList.remove('active');
    }
  });
}

function updateSatelliteVisual(count) {
  const visual = document.getElementById('satelliteVisual');
  if (!visual) return;
  
  visual.innerHTML = '';
  for (let i = 0; i < Math.min(12, Math.max(count, 12)); i++) {
    const indicator = document.createElement('div');
    indicator.className = 'sat-indicator';
    if (i < count) {
      indicator.classList.add('active');
    }
    visual.appendChild(indicator);
  }
}

// Enhanced keyboard controls with visual feedback
document.addEventListener('keydown', function(event) {
  // Prevent default for space and arrow keys
  if ([' ', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(event.key)) {
    event.preventDefault();
  }
  
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
      stopRobot();
      break;
  }
});

function toggleObstacleDetection() {
  fetch('/obstacle-toggle')
    .then(response => {
      if (response.ok) {
        updateConnectionStatus(true);
        addSystemLog('Obstacle detection toggled');
      }
    })
    .catch(error => {
      console.error('Obstacle toggle failed:', error);
      updateConnectionStatus(false);
    });
}

function updateObstacleStatus(data) {
  // Update forward obstacle status
  const forwardStatus = document.getElementById('forwardStatus');
  const forwardIndicator = document.querySelector('#forwardIndicator .indicator-dot');
  
  if (forwardStatus && forwardIndicator) {
    forwardStatus.textContent = data.forwardBlocked ? 'Blocked' : 'Clear';
    forwardIndicator.className = data.forwardBlocked ? 'indicator-dot detected' : 'indicator-dot clear';
  }
  
  // Update backward obstacle status
  const backwardStatus = document.getElementById('backwardStatus');
  const backwardIndicator = document.querySelector('#backwardIndicator .indicator-dot');
  
  if (backwardStatus && backwardIndicator) {
    backwardStatus.textContent = data.backwardBlocked ? 'Blocked' : 'Clear';
    backwardIndicator.className = data.backwardBlocked ? 'indicator-dot detected' : 'indicator-dot clear';
  }
  
  // Update obstacle detection toggle button
  const toggleBtn = document.getElementById('obstacleToggleBtn');
  const toggleText = toggleBtn.querySelector('.btn-text');
  
  if (data.obstacleDetectionEnabled) {
    toggleBtn.className = 'obstacle-toggle-btn enabled';
    toggleText.textContent = 'OBSTACLE DETECTION: ON';
  } else {
    toggleBtn.className = 'obstacle-toggle-btn disabled';
    toggleText.textContent = 'OBSTACLE DETECTION: OFF';
  }
}

function updateSystemStatus(motorStatus) {
  const motorStatusEl = document.getElementById('motorStatus');
  
  if (motorStatusEl) {
    motorStatusEl.textContent = motorStatus || 'Ready';
  }
}

// Add click handlers for control buttons
document.addEventListener('DOMContentLoaded', function() {
  // Control button handlers
  document.querySelectorAll('.control-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      const direction = this.dataset.direction;
      switch(direction) {
        case 'forward': moveForward(); break;
        case 'left': moveLeft(); break;
        case 'right': moveRight(); break;
        case 'reverse': moveReverse(); break;
        case 'stop': stopRobot(); break;
      }
    });
  });
  
  // Speed slider handler
  const motorSlider = document.getElementById('motorSlider');
  if (motorSlider) {
    motorSlider.addEventListener('input', function() {
      updateMotorSpeed(this.value);
    });
    // Set initial speed
    updateMotorSpeed(motorSlider.value);
  }
  
  // System status updates are handled in telemetry
  
  // Mode toggle handlers
  document.querySelectorAll('.mode-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      document.querySelectorAll('.mode-btn').forEach(b => b.classList.remove('active'));
      this.classList.add('active');
    });
  });
});

// Keyboard help functionality
function toggleKeyboardHelp() {
  const help = document.getElementById('keyboardHelp');
  if (help.style.display === 'flex') {
    help.style.display = 'none';
  } else {
    help.style.display = 'flex';
  }
}

// Loading screen management
function hideLoadingScreen() {
  const loading = document.getElementById('loadingOverlay');
  loading.style.opacity = '0';
  setTimeout(() => {
    loading.style.display = 'none';
    document.body.classList.add('fade-in');
  }, 500);
}

// Initialize application
document.addEventListener('DOMContentLoaded', function() {
  // Hide loading screen after initial load
  setTimeout(hideLoadingScreen, 1500);
  
  // Start telemetry updates
  updateTelemetry();
  setInterval(updateTelemetry, 1000);
  
  // Initialize system log with welcome message
  setTimeout(() => {
    addSystemLog('ESP32 Motor Car initialized');
    addSystemLog('Obstacle detection active');
    addSystemLog('System ready for operation');
  }, 2000);
  
  // Add smooth scrolling to all internal links
  document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
      e.preventDefault();
      const target = document.querySelector(this.getAttribute('href'));
      if (target) {
        target.scrollIntoView({ behavior: 'smooth' });
      }
    });
  });
});

// Handle visibility change to pause/resume updates
document.addEventListener('visibilitychange', function() {
  if (document.hidden) {
    // Page is hidden, could pause updates
    console.log('Page hidden - maintaining updates');
  } else {
    // Page is visible, ensure updates are running
    updateTelemetry();
  }
});

// Add error handling for network issues
window.addEventListener('online', function() {
  updateConnectionStatus(true);
  addSystemLog('Network connection restored');
});

window.addEventListener('offline', function() {
  updateConnectionStatus(false);
  addSystemLog('Network connection lost');
});

// Performance optimization: debounce rapid key presses
let keyDebounce = {};
function debounceKey(key, func, delay = 100) {
  clearTimeout(keyDebounce[key]);
  keyDebounce[key] = setTimeout(func, delay);
}

// Add haptic feedback for mobile devices
function addHapticFeedback() {
  if ('vibrate' in navigator) {
    navigator.vibrate(50);
  }
}

// Enhanced button interactions
document.addEventListener('DOMContentLoaded', function() {
  document.querySelectorAll('.control-btn').forEach(btn => {
    btn.addEventListener('touchstart', addHapticFeedback);
    btn.addEventListener('mousedown', function() {
      this.style.transform = 'translateY(-1px) scale(0.98)';
    });
    btn.addEventListener('mouseup', function() {
      this.style.transform = '';
    });
  });
});