## Approach

Use existing PowerShell and Git commands to collect identity without changing
the checkout. Keep linker failures separate from encoding and packaging errors.
Do not publish a supported hash or suggest disabling wrappers without validation.

## Validation

Check the referenced paths and symbols against this revision, exercise the hash
command on a local XBE, and run `git diff --check`. No game build or runtime claim.
