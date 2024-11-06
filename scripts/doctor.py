#!/usr/bin/env python3
# scripts/doctor.py

import sys
import os
import subprocess
import shutil
import socket
import platform
import time
import argparse
from pathlib import Path

class DependencyChecker:
    def __init__(self, verbose=False):
        self.issues = []
        self.warnings = []
        self.suggestions = []
        self.verbose = verbose
        self.min_disk_space_gb = 10
        self.min_memory_gb = 4

    def log(self, message):
        """Log message if in verbose mode"""
        if self.verbose:
            print(f"-> {message}")

    def check_command(self, command, min_version=None, docs_url=None):
        """Check if a command is available and optionally verify its version."""
        self.log(f"Checking command: {command}")

        if not shutil.which(command):
            self.issues.append(f"❌ {command} not found")
            if docs_url:
                self.suggestions.append(f"📝 Install {command}: {docs_url}")
                self.suggestions.append(f"🔍 Search: 'how to install {command} on {platform.system().lower()}'")
            return False

        if min_version:
            try:
                version = subprocess.check_output([command, "--version"]).decode()
                self.log(f"Found {command} version: {version.strip()}")
                if min_version not in version:
                    self.warnings.append(f"⚠️ {command} version might be too old (found: {version.strip()}, want: {min_version})")
                    if docs_url:
                        self.suggestions.append(f"📝 Upgrade guide: {docs_url}")
            except subprocess.CalledProcessError:
                self.warnings.append(f"⚠️ Could not determine {command} version")
        return True

    def check_docker_service(self, service, port):
        """Check if a Docker service is running and responding."""
        self.log(f"Checking Docker service: {service} on port {port}")

        try:
            result = subprocess.run(
                ["docker", "ps", "--filter", f"name={service}", "--format", "{{.Status}}"],
                capture_output=True,
                text=True
            )
            if "Up" not in result.stdout:
                self.issues.append(f"❌ Docker service {service} is not running")
                self.suggestions.append(f"📝 Start service: docker-compose up -d {service}")
                self.suggestions.append(f"📝 Check logs: docker-compose logs {service}")
                return False

            self.log(f"Service {service} is running, checking port {port}")
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex(('localhost', port))
            sock.close()

            if result != 0:
                self.issues.append(f"❌ Service {service} not responding on port {port}")
                self.suggestions.append(f"📝 Check service health: docker inspect {service}")
                self.suggestions.append(f"📝 View logs: docker logs {service}")
                self.suggestions.append(f"🔍 Search: '{service} docker port {port} not accessible'")
                return False

            self.log(f"Service {service} is healthy")
            return True
        except Exception as e:
            self.issues.append(f"❌ Error checking {service}: {str(e)}")
            return False

    def check_system_resources(self):
        """Check system resources like disk space and memory."""
        self.log("Checking system resources")

        # Check disk space
        try:
            if platform.system() == 'Windows':
                free_space = shutil.disk_usage('.').free / (1024**3)
            else:
                stat = os.statvfs('.')
                free_space = (stat.f_bavail * stat.f_frsize) / (1024**3)

            self.log(f"Free disk space: {free_space:.1f}GB")
            if free_space < self.min_disk_space_gb:
                self.issues.append(f"❌ Low disk space: {free_space:.1f}GB free")
                self.suggestions.append(f"📝 Need at least {self.min_disk_space_gb}GB free space")
                self.suggestions.append("📝 Run: docker system prune")
                self.suggestions.append("🔍 Search: 'how to free up disk space linux/windows'")
        except Exception as e:
            self.warnings.append(f"⚠️ Could not check disk space: {e}")

    def check_network_connectivity(self):
        """Check connectivity to essential external services."""
        self.log("Checking network connectivity")

        external_services = {
            'github.com': ('Git repositories', 443),
            'registry.npmjs.org': ('NPM packages', 443),
            'pypi.org': ('Python packages', 443),
            'cdn.jsdelivr.net': ('CDN services', 443)
        }

        for host, (service_name, port) in external_services.items():
            self.log(f"Checking connectivity to {host}:{port} ({service_name})")
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(1)
                result = sock.connect_ex((host, port))
                sock.close()

                if result != 0:
                    self.warnings.append(f"⚠️ Cannot connect to {host} ({service_name})")
                    self.suggestions.append(f"📝 Check if {host} is blocked by firewall")
                    self.suggestions.append(f"📝 Try: ping {host}")
                    self.suggestions.append(f"🔍 Search: '{host} connection refused {platform.system().lower()}'")
            except Exception as e:
                self.warnings.append(f"⚠️ Failed to check {host}: {str(e)}")

    def run_checks(self):
        """Run all checks and print results."""
        print("\n=== Development Environment Check ===\n")

        # Core tools with documentation links
        tools = {
            "cmake": ("3.15", "https://cmake.org/download/"),
            "git": ("2.0", "https://git-scm.com/downloads"),
            "docker": ("20.0", "https://docs.docker.com/get-docker/"),
            "docker-compose": ("1.29", "https://docs.docker.com/compose/install/"),
            "gcc": ("9.0", "https://gcc.gnu.org/install/"),
            "clang": ("10.0", "https://releases.llvm.org/"),
            "python3": ("3.8", "https://www.python.org/downloads/"),
            "node": ("14.0", "https://nodejs.org/"),
            "npm": ("6.0", "https://www.npmjs.com/get-npm")
        }

        self.log("Starting core tool checks...")
        for tool, (version, url) in tools.items():
            self.check_command(tool, version, url)

        # Service checks
        self.log("\nChecking Docker services...")
        services = {
            "postgres": (5432, "https://hub.docker.com/_/postgres"),
            "mongodb": (27017, "https://hub.docker.com/_/mongo"),
            "redis": (6379, "https://hub.docker.com/_/redis"),
            "rabbitmq": (5672, "https://hub.docker.com/_/rabbitmq"),
            "kafka": (9092, "https://hub.docker.com/r/confluentinc/cp-kafka"),
            "zookeeper": (2181, "https://hub.docker.com/_/zookeeper")
        }

        for service, (port, docs_url) in services.items():
            if not self.check_docker_service(service, port):
                self.suggestions.append(f"📝 Service docs: {docs_url}")

        # Additional checks
        self.log("\nPerforming system checks...")
        self.check_system_resources()
        self.check_network_connectivity()

        # Print results
        if not self.issues and not self.warnings:
            print("✅ All checks passed!")
            return True

        if self.issues:
            print("\n🚨 Issues Found:")
            for issue in self.issues:
                print(f"  {issue}")

        if self.warnings:
            print("\n⚠️ Warnings:")
            for warning in self.warnings:
                print(f"  {warning}")

        if self.suggestions:
            print("\n💡 Suggestions:")
            for suggestion in self.suggestions:
                print(f"  {suggestion}")

        print("\n📚 For more help:")
        print("  - Project documentation: https://github.com/your-org/project/wiki")
        print("  - Common issues: https://github.com/your-org/project/wiki/Common-Issues")
        print("  - Developer setup guide: https://github.com/your-org/project/wiki/Developer-Setup")

        return len(self.issues) == 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Check development environment setup')
    parser.add_argument('-v', '--verbose', action='store_true', help='Show detailed progress')
    args = parser.parse_args()

    checker = DependencyChecker(verbose=args.verbose)
    sys.exit(0 if checker.run_checks() else 1)
