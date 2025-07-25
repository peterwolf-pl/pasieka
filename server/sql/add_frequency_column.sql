-- Add frequency column for microphone data
ALTER TABLE measurements ADD COLUMN frequency FLOAT DEFAULT NULL AFTER weight;
