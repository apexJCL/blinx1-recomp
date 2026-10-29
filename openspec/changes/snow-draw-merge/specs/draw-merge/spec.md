## ADDED Requirements

### Requirement: Back-to-back triangle batches draw as one list
On a backend with `merge_draws` and `display.aspect` stock, the walker SHALL
hold a batch made only of DRAW_ARRAYS or ARRAY_ELEMENT indices of a triangle
primitive at its END, add the batches that follow with the same primitive and
no other method in between, and draw them as one TRIANGLES list holding the
triangles each batch would have drawn, in order.

#### Scenario: A chain of strips
- **WHEN** three triangle-strip batches follow one another with no method between and then a state method arrives
- **THEN** the backend gets one TRIANGLES draw of the three strips' triangles before that method is applied, and the draw counter counts three batches

#### Scenario: A lone batch
- **WHEN** a held batch is followed by any other method
- **THEN** it reaches the backend with its own primitive and indices

#### Scenario: Walk end
- **WHEN** a walk ends with batches held
- **THEN** they are drawn before GET moves

### Requirement: Merging can be turned off
`RECOMP_DEBUG=pb_merge=0`, `display.aspect` set, or a backend without
`merge_draws` SHALL draw every batch on its own.

#### Scenario: Hor+
- **WHEN** `display.aspect` is 16:10
- **THEN** every batch reaches the backend on its own
