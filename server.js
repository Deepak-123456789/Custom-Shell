const { spawn } = require('child_process');
const WebSocket = require('ws');
const path = require('path');
const fs = require('fs');

const wss = new WebSocket.Server({ port: 3000 });

const getPasswordFromEnvFile = () => {
  const envPath = path.join(__dirname, '.env');
  const envContent = fs.readFileSync(envPath, 'utf-8');
  const match = envContent.match(/^SHELL_PASSWORD=(.*)$/m);
  return match ? match[1].trim() : null;
};

wss.on('connection', (ws) => {
  const password = getPasswordFromEnvFile();
  let authenticated = false;
  let shellProcess = null;
  let waitingForPassword = true;

  ws.send('Connected to shell server.\nPlease enter password:\n');

  ws.on('message', (message) => {
    const msg = message.toString().trim();

    if (!authenticated) {
      // Check password from client
      if (msg === password) {
        authenticated = true;
        ws.send('Authentication successful! Welcome to the shell.\n');

        // Spawn shell backend process
        shellProcess = spawn('./myshell', ['--frontend'], {
          stdio: ['pipe', 'pipe', 'pipe'],
          env: process.env,
        });

        // Send the password to shell process stdin to pass its own auth check
        shellProcess.stdin.write(password + '\n');

        // Pipe shell stdout and stderr back to client
        shellProcess.stdout.on('data', (data) => ws.send(data.toString()));
        shellProcess.stderr.on('data', (data) => ws.send(data.toString()));

        shellProcess.on('close', (code) => {
          ws.send(`Shell exited with code ${code}\n`);
          ws.close();
        });
      } else {
        ws.send('Authentication failed. Please enter password again:\n');
      }
      return;
    }

    // After auth, send commands to shell process stdin
    if (shellProcess) {
      // Prevent sending the password again as a command to shell
      if (msg !== password && msg.length > 0) {
        shellProcess.stdin.write(msg + '\n');
      }
    }
  });

  ws.on('close', () => {
    if (shellProcess) {
      shellProcess.kill();
    }
    console.log('Client disconnected');
  });
});

console.log('WebSocket server running on ws://localhost:3000');
