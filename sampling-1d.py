import click
import matplotlib.pyplot as plt
import numpy as np

from scipy.fftpack import dct, idct

@click.command()
@click.option('--num-samples', type=int, default=100)
def sample(num_samples):

    original_signal = np.linspace(0, 1, 100)

    basis_functions = idct(np.identity(original_signal.shape[0]))

    dct_weights = np.zeros_like(original_signal)

    for i in range(num_samples):
        sample_point = int(np.random.uniform(0, original_signal.shape[0]))
        original_value = original_signal[sample_point]

        approx_signal = idct(dct_weights)
        approx_value = approx_signal[sample_point]

        delta = original_value - approx_value

        basis_values = basis_functions[:, sample_point]
        basis_strength = np.sum(basis_values)
        dct_weights += 0.005 * delta * basis_values

    plt.plot(original_signal)
    plt.plot(idct(dct_weights))
    plt.show()

if __name__ == '__main__':
    sample()
