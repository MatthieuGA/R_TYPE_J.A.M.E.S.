#!/bin/bash

# Local Epitech Coding Style Checker
# This script runs the same checks as the CI

set -e

REPO_DIR="$(pwd)"
REPORT_FILE="$REPO_DIR/coding-style-reports.log"

echo "🔍 Running Epitech Coding Style Checker..."
echo "Repository: $REPO_DIR"
echo ""

# Clean up old report
rm -f "$REPORT_FILE"
touch "$REPORT_FILE"
chmod 666 "$REPORT_FILE"

# Run the checker
docker run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$REPO_DIR:/mnt/delivery:ro" \
  -v "$REPO_DIR:/mnt/reports:rw" \
  ghcr.io/epitech/coding-style-checker:latest \
  /mnt/delivery /mnt/reports 2>&1 || true

# Check if report exists and has content
if [ ! -f "$REPORT_FILE" ]; then
  echo "❌ Report file not generated!"
  exit 1
fi

if [ ! -s "$REPORT_FILE" ]; then
  echo "✅ No coding style errors found!"
  exit 0
fi

# Parse and display errors
echo "📋 Coding Style Errors Found:"
echo "=============================="
echo ""

error_count=0
while IFS= read -r line; do
  if [ -n "$line" ]; then
    file=$(echo "$line" | cut -d':' -f1)
    nbline=$(echo "$line" | cut -d':' -f2)
    error_type=$(echo "$line" | cut -d':' -f3)
    description=$(echo "$line" | cut -d':' -f4)
    
    echo "❌ $file:$nbline"
    echo "   Type: $error_type"
    echo "   Info: $description"
    echo ""
    
    error_count=$((error_count + 1))
  fi
done < "$REPORT_FILE"

echo "=============================="
echo "Total errors: $error_count"
echo ""

if [ "$error_count" -gt 0 ]; then
  echo "❌ Coding style check FAILED"
  echo ""
  echo "Full report saved to: $REPORT_FILE"
  exit 1
else
  echo "✅ Coding style check PASSED"
  exit 0
fi
