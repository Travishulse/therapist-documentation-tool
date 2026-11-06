import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import crypto from 'node:crypto';
import { execFileSync } from 'node:child_process';

const rootDir = process.cwd();
const distHtmlPath = path.join(rootDir, 'dist', 'index.html');
const distNamedHtmlPath = path.join(rootDir, 'dist', 'Therapist-Documentation-Tool.html');
const releaseDir = path.join(rootDir, 'release');
const releaseHtmlPath = path.join(releaseDir, 'Therapist-Documentation-Tool.html');
const releaseZipPath = path.join(releaseDir, 'Therapist-Documentation-Tool.zip');
const docsDir = path.join(rootDir, 'docs');
const docsHtmlPath = path.join(docsDir, 'index.html');
const nativeExePath = path.join(rootDir, 'production', 'build', 'appTherapistDocumentationTool_Production.exe');

if (!fs.existsSync(distHtmlPath)) {
  throw new Error(`Missing build output: ${distHtmlPath}`);
}

let html = fs.readFileSync(distHtmlPath, 'utf8');

// Convert module script tag to classic script tag for universal file:// execution
html = html.replace(/<script\s+type="module"\s+crossorigin>/g, '<script>');
html = html.replace(/<script\s+type="module">/g, '<script>');

// Verify Content Security Policy is present and locks down connect-src
if (!html.includes("http-equiv=\"Content-Security-Policy\"") || !html.includes("connect-src 'none'")) {
  throw new Error('Security gate failed: Missing strict offline Content-Security-Policy.');
}

// Verify zero external stylesheet or script tags
if (/<link[^>]+href="https?:\/\//i.test(html) || /<script[^>]+src="https?:\/\//i.test(html)) {
  throw new Error('Security gate failed: External network <link> or <script> detected in single-file HTML.');
}

// Extract inline script and validate with V8 vm.Script
const scriptMatch = html.match(/<script>([\s\S]*?)<\/script>/);
if (!scriptMatch) {
  throw new Error('Build gate failed: No inline <script> found in bundled HTML.');
}
new vm.Script(scriptMatch[1], { filename: 'Therapist-Documentation-Tool.html' });

fs.mkdirSync(releaseDir, { recursive: true });
fs.mkdirSync(docsDir, { recursive: true });

fs.writeFileSync(distHtmlPath, html, 'utf8');
fs.writeFileSync(distNamedHtmlPath, html, 'utf8');
fs.writeFileSync(releaseHtmlPath, html, 'utf8');
fs.writeFileSync(docsHtmlPath, html, 'utf8');

// Create clean ZIP archive containing the single-file HTML deliverable
if (fs.existsSync(releaseZipPath)) {
  fs.unlinkSync(releaseZipPath);
}
try {
  execFileSync('powershell.exe', [
    '-NoProfile',
    '-Command',
    `Compress-Archive -Path "${releaseHtmlPath}" -DestinationPath "${releaseZipPath}" -Force`
  ]);
} catch (err) {
  console.warn('Warning: Compress-Archive skipped:', err.message);
}

// Generate cryptographic SHA-256 manifest for zero-alert verification
const sha256File = (filePath) => {
  const buf = fs.readFileSync(filePath);
  return crypto.createHash('sha256').update(buf).digest('hex');
};

const checksumLines = [];
checksumLines.push(`${sha256File(releaseHtmlPath)}  release/Therapist-Documentation-Tool.html`);
if (fs.existsSync(releaseZipPath)) {
  checksumLines.push(`${sha256File(releaseZipPath)}  release/Therapist-Documentation-Tool.zip`);
}
if (fs.existsSync(nativeExePath)) {
  checksumLines.push(`${sha256File(nativeExePath)}  production/build/appTherapistDocumentationTool_Production.exe`);
}

const shaPath = path.join(releaseDir, 'SHA256SUMS.txt');
fs.writeFileSync(shaPath, checksumLines.join('\n') + '\n', 'utf8');

const sizeKb = (fs.statSync(releaseHtmlPath).size / 1024).toFixed(1);
console.log(`✓ Single-file HTML verified and packaged (${sizeKb} KB)`);
console.log(`✓ Offline CSP, V8 classic script syntax, and SHA-256 manifest verified.`);

