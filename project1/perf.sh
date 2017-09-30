sudo perf record -g ~/ref_codes/marker/marker ~/ref_codes/small/small.init ~/ref_codes/small/small.work ~/ref_codes/small/small.result ./run
# sudo perf record -g ~/ref_codes/marker/marker ~/ref_codes/marker/tiny.init ~/ref_codes/marker/tiny.work ~/ref_codes/marker/tiny.result ./run
sudo perf report -g graph --no-children
