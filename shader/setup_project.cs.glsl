layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void compute_divergence();
void build_a();

void main() {
    compute_divergence();
    build_a();
}
