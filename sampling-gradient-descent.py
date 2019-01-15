import click
import cv2
import matplotlib.pyplot as plt
import numpy as np

from PIL import Image
from scipy.fftpack import dct, idct

def dct_image(image):
    return dct(dct(image.T).T)

def idct_image(weights):
    return idct(idct(weights.T).T)

TEMP_VALUE = np.ndarray([])

def integrate_sample(row_basis_functions,
                     col_basis_functions,
                     dct_weights,
                     sample_point,
                     sample):

    # Evaluate all of the row and column basis functions at the sample point
    row_basis_values = row_basis_functions[:,sample_point[0]]
    col_basis_values = col_basis_functions[:,sample_point[1]]

    # basis_values[i,j] = row_basis_values[i] * col_basis_values[j]
    TEMP_VALUE.resize(dct_weights.shape)
    basis_values = np.outer(row_basis_values, col_basis_values, out=TEMP_VALUE)

    delta = sample - dct_weights * basis_values

    basic_step = 0.000005

    # Adjust the existing dct weights by the observed delta
    dct_weights += basic_step * delta * basis_values

@click.command()
@click.option('--image', required=True)
@click.option('--sample-factor', type=float, default=0.5)
def sample(image, sample_factor):

    original_luma = np.asarray(Image.open(image).convert('L')).astype(np.float)
    original_luma = np.interp(original_luma, (0, 255), (0.0, 1.0))

    # Precompute row and column basis functions
    nrows = original_luma.shape[0]
    row_basis_functions = idct(np.identity(nrows), axis=1)
    ncols = original_luma.shape[1]
    col_basis_functions = idct(np.identity(ncols), axis=1)

    # cv2.imshow('Row Basis', np.interp(row_basis_functions, (-1.0, 1.0), (0.0, 1.0)))
    # cv2.waitKey(0)
    # return

    dct_weights = np.zeros_like(original_luma)

    num_samples = int(original_luma.shape[0] * original_luma.shape[1] * sample_factor)
    print(f'Taking {num_samples} samples')

    with click.progressbar(range(num_samples)) as iterable:
        for i in iterable:
            sample_point = (int(np.random.uniform(0, original_luma.shape[0])),
                            int(np.random.uniform(0, original_luma.shape[1])))
            sample_point = (int(np.random.uniform(500, 520)),
                            int(np.random.uniform(500, 520)))

            sample_value = original_luma[sample_point]

            integrate_sample(row_basis_functions,
                             col_basis_functions,
                             dct_weights,
                             sample_point,
                             sample_value)


    reconstructed_luma = idct_image(dct_weights)
    plt.plot(reconstructed_luma[512,:])
    plt.show()

    cv2.imshow('Original image', original_luma)
    cv2.imshow('Reconstructed image', np.clip(reconstructed_luma, 0, 1))
    cv2.waitKey(0)

    cv2.destroyAllWindows()

if __name__ == '__main__':
    sample()
