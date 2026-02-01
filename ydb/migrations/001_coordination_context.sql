-- +goose Up
CREATE TABLE coordination_context (
    partition_id Uint64,
    cooldown_epoch Uint64,
    weight Uint64,
    PRIMARY KEY (partition_id)
)

-- +goose Down
DROP TABLE coordination_context;