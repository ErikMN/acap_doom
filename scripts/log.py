#!/usr/bin/python3
'''
Trace systemd-journald log over ssh

python3-paramiko
python3-scp

'''
import os
import re
import sys
import argparse
import paramiko

# ANSI color codes for terminal text formatting
ANSI_RESET = "\033[0m"           # Reset all text formatting
ANSI_BOLD = "\033[1m"            # Bold text
ANSI_DIM = "\033[2m"             # Dim text
ANSI_UNDERLINE = "\033[4m"       # Underline text
ANSI_BLINK = "\033[5m"           # Blinking text
# Reverse (swap background and foreground colors)
ANSI_REVERSE = "\033[7m"
ANSI_HIDDEN = "\033[8m"          # Hidden text (not visible)
ANSI_STRIKETHROUGH = "\033[9m"   # Strikethrough text

# Foreground text colors
ANSI_BLACK = "\033[30m"
ANSI_RED = "\033[31m"
ANSI_GREEN = "\033[32m"
ANSI_YELLOW = "\033[33m"
ANSI_BLUE = "\033[34m"
ANSI_MAGENTA = "\033[35m"
ANSI_CYAN = "\033[36m"
ANSI_WHITE = "\033[37m"

# Background colors (Add 10 to the foreground color code)
ANSI_BG_BLACK = "\033[40m"
ANSI_BG_RED = "\033[41m"
ANSI_BG_GREEN = "\033[42m"
ANSI_BG_YELLOW = "\033[43m"
ANSI_BG_BLUE = "\033[44m"
ANSI_BG_MAGENTA = "\033[45m"
ANSI_BG_CYAN = "\033[46m"
ANSI_BG_WHITE = "\033[47m"

# High-intensity text colors (Bright or Bold)
ANSI_HI_BLACK = "\033[90m"
ANSI_HI_RED = "\033[91m"
ANSI_HI_GREEN = "\033[92m"
ANSI_HI_YELLOW = "\033[93m"
ANSI_HI_BLUE = "\033[94m"
ANSI_HI_MAGENTA = "\033[95m"
ANSI_HI_CYAN = "\033[96m"
ANSI_HI_WHITE = "\033[97m"

# High-intensity background colors (Add 10 to the high-intensity color code)
ANSI_BG_HI_BLACK = "\033[100m"
ANSI_BG_HI_RED = "\033[101m"
ANSI_BG_HI_GREEN = "\033[102m"
ANSI_BG_HI_YELLOW = "\033[103m"
ANSI_BG_HI_BLUE = "\033[104m"
ANSI_BG_HI_MAGENTA = "\033[105m"
ANSI_BG_HI_CYAN = "\033[106m"
ANSI_BG_HI_WHITE = "\033[107m"


def trace_journalctl_ssh(username, password, args):
    try:
        # Create SSH client:
        ssh_client = paramiko.SSHClient()
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        # Connect to the remote server:
        ssh_client.connect(os.environ['TARGET_IP'],
                           username=username, password=password)

        # Check if the 'journalctl' command is available on the remote server:
        stdin, stdout, stderr = ssh_client.exec_command('which journalctl')
        if stdout.channel.recv_exit_status() != 0:
            print(
                ANSI_RED + "Error: 'journalctl' command is not available on the remote server." + ANSI_RESET)
            ssh_client.close()
            return

        # Build the journalctl command with the provided arguments:
        journalctl_cmd = 'journalctl' + \
            (' -f' if '-f' not in args else '') + ' '.join(args)

        # Start journalctl in follow mode ("-f"):
        stdin, stdout, stderr = ssh_client.exec_command(journalctl_cmd)

        # Read and print the output with color coding:
        for line in stdout:
            log = line.strip()
            # Remove timestamp and hostname:
            log = re.sub(r'^\w{3} \d{2} \d{2}:\d{2}:\d{2} \S+ ', '', log)
            if "ERROR" in log:
                print(ANSI_RED + ANSI_BOLD + log + ANSI_RESET)
            elif "WARN" in log:
                print(ANSI_YELLOW + log + ANSI_RESET)
            elif "kernel" in log:
                print(ANSI_RED + log + ANSI_RESET)
            elif "sdk" in log:
                print(ANSI_BG_GREEN + log + ANSI_RESET)
            elif "acap_doom" in log:
                print(ANSI_BLUE + log + ANSI_RESET)
            elif "sshd" in log:
                print(ANSI_GREEN + log + ANSI_RESET)
            else:
                print(log)

        # Check for any error messages from stderr
        error_output = stderr.read().decode().strip()
        if error_output:
            print(ANSI_RED + "Failed to execute journalctl command:\n" +
                  error_output + ANSI_RESET)

    except paramiko.AuthenticationException:
        print("Authentication failed. Please check the username and password.")
    except paramiko.SSHException as ssh_ex:
        print(f"SSH error: {ssh_ex}")
    except KeyboardInterrupt:
        print("\nTerminating logger script.")
        sys.exit(0)
    except Exception as e:
        print(f"Error: {e}")
    finally:
        ssh_client.close()


if __name__ == "__main__":
    # Check if TARGET_IP environment variable is set:
    if 'TARGET_IP' not in os.environ:
        print(
            "TARGET_IP environment variable is not set. Please set it to the SSH hostname.")
        sys.exit(1)

    # Check if TARGET_USR and TARGET_PWD environment variables are set:
    if 'TARGET_USR' not in os.environ:
        print(
            "TARGET_USR environment variable is not set. Please set it to the SSH username.")
        sys.exit(1)

    if 'TARGET_PWD' not in os.environ:
        print(
            "TARGET_PWD environment variable is not set. Please set it to the SSH password.")
        sys.exit(1)

    # SSH credentials from environment variables:
    SSH_USERNAME = os.environ['TARGET_USR']
    SSH_PASSWORD = os.environ['TARGET_PWD']

    # Parse additional arguments to journalctl using argparse:
    parser = argparse.ArgumentParser()
    parser.add_argument('args', nargs='*')
    args = parser.parse_args().args

    trace_journalctl_ssh(SSH_USERNAME, SSH_PASSWORD, args)
