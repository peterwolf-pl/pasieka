-- Add board_id column to existing measurements table
ALTER TABLE measurements
  ADD COLUMN board_id INT NOT NULL DEFAULT 1 AFTER weight;

-- Update historical records to use board_id = 1
UPDATE measurements SET board_id = 1 WHERE board_id IS NULL;

-- Example: change board ID 1 to 2 for all old records
-- UPDATE measurements SET board_id = 2 WHERE board_id = 1;
